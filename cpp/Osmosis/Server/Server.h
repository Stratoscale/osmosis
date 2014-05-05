#ifndef __OSMOSIS_SERVER_SERVER_H__
#define __OSMOSIS_SERVER_SERVER_H__

#include "Osmosis/Server/Thread.h"

namespace Osmosis {
namespace Server
{

class Server
{
public:
	Server( boost::filesystem::path         rootPath,
		boost::asio::ip::tcp::endpoint  endpoint,
		ObjectStore::Store &            store,
		ObjectStore::Drafts &           drafts,
		ObjectStore::Labels &           labels ) :
		_rootPath( rootPath ),
		_store( store ),
		_drafts( drafts ),
		_labels( labels ),
		_acceptor( _ioService, endpoint )
	{
		boost::asio::socket_base::reuse_address option( true );
		_acceptor.set_option(option);
	}

	void run()
	{
		TRACE_INFO( "Osmosis server up and running, waiting for connections" );
		while ( true ) {
			std::shared_ptr< Thread > thread = std::make_shared< Thread >( _rootPath, _store, _drafts, _labels );
			_acceptor.accept( thread->socketForAccept() );
			thread->run();
		}
	}

private:
	boost::filesystem::path         _rootPath;
	ObjectStore::Store &            _store;
	ObjectStore::Drafts &           _drafts;
	ObjectStore::Labels &           _labels;
	boost::asio::io_service         _ioService;
	boost::asio::ip::tcp::acceptor  _acceptor; 

	Server( const Server & rhs ) = delete;
	Server & operator= ( const Server & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_SERVER_H__
