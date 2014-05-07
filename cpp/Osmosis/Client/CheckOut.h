#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/DigestDrafts.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/ApplyFileStatus.h"

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
			bool                             removeUnknownFiles ) :
		_directory( directory ),
		_label( label ),
		_hostname( hostname ),
		_port( port ),
		_removeUnknownFiles( removeUnknownFiles ),
		_digestDirectory( directory, md5 )
	{}

	void go()
	{
		std::istringstream dirListText( getLabelDirList() );
		DirList labelDirList;
		dirListText >> labelDirList;
		_digestDirectory.join();

		FetchFiles fetchFiles( _directory, _hostname, _port );
		for ( auto & entry : labelDirList.entries() ) {
			boost::filesystem::path absolute = _directory / entry.path;
			auto digestedEntry = _digestDirectory.dirList().find( entry.path );
			if ( digestedEntry == nullptr ) {
				if ( entry.status.syncContent() ) {
					if ( not entry.hash )
						THROW( Error, "No hash for file that should have data - directory listing is defective" );
					fetchFiles.fetch( entry.path, entry.status, * entry.hash );
				} else
					ApplyFileStatus( absolute, entry.status ).createNonRegular();
			} else {
				if ( entry.status.syncContent() ) {
					if ( digestedEntry->status.syncContent() ) {
						if ( * entry.hash != * digestedEntry->hash )
							fetchFiles.fetch( entry.path, entry.status, * entry.hash );
						else if ( entry.status != digestedEntry->status ) {
							ApplyFileStatus( absolute, entry.status ).applyExistingRegular();
							ASSERT( FileStatus( absolute ) == entry.status );
						}
					} else {
						boost::filesystem::remove( absolute );
						fetchFiles.fetch( entry.path, entry.status, * entry.hash );
					}
				} else {
					if ( entry.status != digestedEntry->status ) {
						boost::filesystem::remove( absolute );
						ApplyFileStatus( absolute, entry.status ).createNonRegular();
					}
				}
			}
		}
		fetchFiles.noMoreFilesToFetch();
		removeUnknownFiles( _digestDirectory.dirList(), labelDirList );
		fetchFiles.join();
		TRACE_DEBUG( "Checkout Complete" );
	}

private:
	const boost::filesystem::path  _directory;
	const std::string              _label;
	const std::string              _hostname;
	const unsigned short           _port;
	const bool                     _removeUnknownFiles;
	DigestDirectory                _digestDirectory; 

	std::string getLabelDirList()
	{
		Connect connect( _hostname, _port );
		Hash hash = LabelOps( connect.socket() ).get( _label );
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

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
