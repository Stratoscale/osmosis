#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/DigestDrafts.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/Client/DelayedLabels.h"
#include "Osmosis/Client/FetchJointDirlistFromLabels.h"
#include "Osmosis/ApplyFileStatus.h"
#include "Osmosis/OSUtils.h"

namespace Osmosis {
namespace Client
{

class CheckOut
{
public:
	CheckOut(       const boost::filesystem::path &  directory,
			const std::string &              label,
			const std::string &              hostname,
			unsigned short                   port,
			bool                             md5,
			bool                             removeUnknownFiles,
			bool                             myUIDandGIDcheckout ) :
		_directory( directory ),
		_labels( label ),
		_hostname( hostname ),
		_port( port ),
		_removeUnknownFiles( removeUnknownFiles ),
		_myUIDandGIDcheckout( myUIDandGIDcheckout ),
		_digestDirectory( directory, md5 )
	{}

	void go()
	{
		_labels.fetch();
		DirList labelsDirList( FetchJointDirlistFromLabels( _labels.labels(), _hostname, _port ).joined() );
		_digestDirectory.join();

		FetchFiles fetchFiles( _directory, _hostname, _port );
		for ( auto & entry : labelsDirList.entries() )
			if ( _myUIDandGIDcheckout ) {
				FileStatus modifiedStatus( entry.status );
				modifiedStatus.setUIDGID( OSUtils::uid(), OSUtils::gid() );
				decideWhatToDo( fetchFiles,
						entry.path,
						modifiedStatus,
						entry.hash.get(),
						labelsDirList );
			} else
				decideWhatToDo( fetchFiles,
						entry.path,
						entry.status,
						entry.hash.get(),
						labelsDirList );
		fetchFiles.noMoreFilesToFetch();
		removeUnknownFiles( _digestDirectory.dirList(), labelsDirList );
		fetchFiles.join();
		TRACE_DEBUG( "Checkout Complete" );
	}

private:
	const boost::filesystem::path  _directory;
	DelayedLabels                  _labels;
	const std::string              _hostname;
	const unsigned short           _port;
	const bool                     _removeUnknownFiles;
	const bool                     _myUIDandGIDcheckout;
	DigestDirectory                _digestDirectory;

	void removeUnknownFiles( const DirList & digested, const DirList & label )
	{
		if ( not _removeUnknownFiles )
			return;
		for ( auto entry = digested.entries().rbegin();
				entry != digested.entries().rend(); ++ entry )
			if ( label.find( entry->path ) == nullptr ) {
				boost::filesystem::path absolute = _directory / entry->path;
				boost::filesystem::remove( absolute );
			}
	}

	void decideWhatToDo(    FetchFiles &                     fetchFiles,
				const boost::filesystem::path &  path,
				const FileStatus &               status,
				const Hash *                     hash,
				const DirList &                  labelDirList )
	{
		boost::filesystem::path absolute = _directory / path;
		auto digestedEntry = _digestDirectory.dirList().find( path );
		if ( digestedEntry == nullptr ) {
			if ( status.syncContent() ) {
				if ( hash == nullptr )
					THROW( Error, "No hash for file that should have data - directory listing is defective" );
				fetchFiles.fetch( path, status, * hash );
			} else
				ApplyFileStatus( absolute, status ).createNonRegular();
		} else {
			if ( status.syncContent() ) {
				if ( digestedEntry->status.syncContent() ) {
					if ( hash == nullptr )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					if ( ! digestedEntry->hash )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					if ( * hash != * digestedEntry->hash )
						fetchFiles.fetch( path, status, * hash );
					else if ( status != digestedEntry->status ) {
						ApplyFileStatus( absolute, status ).applyExistingRegular();
						ASSERT_VERBOSE( FileStatus( absolute ) == status,
								absolute << ": " << FileStatus( absolute ) << " != " << status );
					}
				} else {
					if ( hash == nullptr )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					boost::filesystem::remove( absolute );
					fetchFiles.fetch( path, status, * hash );
				}
			} else {
				if ( status != digestedEntry->status )
					ApplyFileStatus( absolute, status ).applyNonRegular( digestedEntry->status );
			}
		}
	}

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
