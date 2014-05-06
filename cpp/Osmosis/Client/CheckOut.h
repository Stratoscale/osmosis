#ifndef __OSMOSIS_CLIENT_CHECK_OUT_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_H__

#include "Osmosis/Stream/SocketToBuffer.h"
#include "Osmosis/Client/DigestDrafts.h"

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
		_digestDirectory( directory, md5 ),
		_getConnection( hostname, port )
	{}

	void go()
	{
		std::istringstream dirListText( getLabelDirList() );
		DirList labelDirList;
		dirListText >> labelDirList;
		_digestDirectory.join();

		ObjectStore::Drafts drafts( _directory );
		try {
			for ( auto & entry : labelDirList.entries() ) {
				auto digestedEntry = _digestDirectory.dirList().find( entry.path );
				if ( digestedEntry == nullptr ) {
ASSERT( entry.hash );
					fetch( entry, drafts );
				} else {
ASSERT( entry.hash );
ASSERT( digestedEntry->hash );
					if ( * entry.hash != * digestedEntry->hash )
						fetch( entry, drafts );
				}
			}
		} catch ( ... ) {
			try {
				drafts.eraseDirectory();
			} CATCH_ALL_IGNORE( "Unable to erase drafts directory" );
			throw;
		}
		_digestDrafts.toDigestTaskQueue().producerDone();

		try {
			while ( true ) {
				auto task = _digestDrafts.digestedTaskQueue().get();
				boost::filesystem::rename( task.draft, task.path );
			}
		} catch ( ToVerifyTaskQueue::NoMoreTasksError & ) {
			TRACE_DEBUG( "Finished moving drafts into place" );
		}
		_digestDrafts.join();
		drafts.eraseDirectory();
		TRACE_DEBUG( "Checkout Complete" );
	}

private:
	const boost::filesystem::path  _directory;
	const std::string              _label;
	DigestDirectory                _digestDirectory;
	Connect                        _getConnection;
	DigestDrafts                   _digestDrafts;

	std::string getLabelDirList()
	{
		Hash hash = LabelOps( _getConnection.socket() ).get( _label );
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
		_getConnection.socket().sendAllConcated( header, hash.raw() );
		Stream::SocketToBuffer transfer( _getConnection.socket() );
		transfer.transfer();
		TRACE_DEBUG( "Transferred dirList" );
		return std::move( transfer.data() );
	}

	void getObject( const boost::filesystem::path & path, const Hash & hash )
	{
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
		_getConnection.socket().sendAllConcated( header, hash.raw() );
		Stream::SocketToFile transfer( _getConnection.socket(), path.string().c_str() );
		transfer.transfer();
		TRACE_DEBUG( "Transferred " << hash );
	}

	void fetch( const DirListEntry & entry, ObjectStore::Drafts & drafts )
	{
		boost::filesystem::path draft = drafts.allocateFilename();
		getObject( draft, * entry.hash );
		ToVerify task = { entry.path, * entry.hash, draft };
		_digestDrafts.toDigestTaskQueue().put( std::move( task ) );
	}

	CheckOut( const CheckOut & rhs ) = delete;
	CheckOut & operator= ( const CheckOut & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_H__
