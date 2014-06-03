#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/DigestDrafts.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/Client/DelayedLabel.h"
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
		_label( label ),
		_hostname( hostname ),
		_port( port ),
		_removeUnknownFiles( removeUnknownFiles ),
		_myUIDandGIDcheckout( myUIDandGIDcheckout ),
		_digestDirectory( directory, md5 )
	{}

	void go()
	{
		_label.fetch();
		std::istringstream dirListText( getLabelDirList() );
		DirList labelDirList;
		dirListText >> labelDirList;
		_digestDirectory.join();

		FetchFiles fetchFiles( _directory, _hostname, _port );
		for ( auto & entry : labelDirList.entries() )
			if ( _myUIDandGIDcheckout ) {
				FileStatus modifiedStatus( entry.status );
				modifiedStatus.setUIDGID( OSUtils::uid(), OSUtils::gid() );
				decideWhatToDo( fetchFiles,
						entry.path,
						modifiedStatus,
						entry.hash.get(),
						labelDirList );
			} else
				decideWhatToDo( fetchFiles,
						entry.path,
						entry.status,
						entry.hash.get(),
						labelDirList );
		fetchFiles.noMoreFilesToFetch();
		removeUnknownFiles( _digestDirectory.dirList(), labelDirList );
		fetchFiles.join();
		TRACE_DEBUG( "Checkout Complete" );
	}

private:
	const boost::filesystem::path  _directory;
	DelayedLabel                   _label;
	const std::string              _hostname;
	const unsigned short           _port;
	const bool                     _removeUnknownFiles;
	const bool                     _myUIDandGIDcheckout;
	DigestDirectory                _digestDirectory;

	std::string getLabelDirList()
	{
		Connect connect( _hostname, _port );
		Hash hash = LabelOps( connect.socket() ).get( _label.label() );
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
		connect.socket().sendAllConcated( header, hash.raw() );
		Stream::SocketToBuffer transfer( connect.socket() );
		transfer.transfer();
		std::string result = transfer.data();
		if ( not CalculateHash::verify( result.c_str(), result.size(), hash ) )
			THROW( Error, "Dir list hash did not match contents" );
		TRACE_DEBUG( "Transferred dirList" );
		return std::move( result );
	}

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
