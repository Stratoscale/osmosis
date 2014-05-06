#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/DigestDrafts.h"
#include "Osmosis/Client/FetchFiles.h"

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
			bool                             md5 ) :
		_directory( directory ),
		_label( label ),
		_hostname( hostname ),
		_port( port ),
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
			auto digestedEntry = _digestDirectory.dirList().find( entry.path );
			if ( digestedEntry == nullptr ) {
ASSERT( entry.hash );
				fetchFiles.fetch( entry.path, * entry.hash );
			} else {
ASSERT( entry.hash );
ASSERT( digestedEntry->hash );
				if ( * entry.hash != * digestedEntry->hash )
					fetchFiles.fetch( entry.path, * entry.hash );
			}
		}
		fetchFiles.noMoreFilesToFetch();
		fetchFiles.join();
		TRACE_DEBUG( "Checkout Complete" );
	}

private:
	const boost::filesystem::path  _directory;
	const std::string              _label;
	const std::string              _hostname;
	unsigned short                 _port; 
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

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
