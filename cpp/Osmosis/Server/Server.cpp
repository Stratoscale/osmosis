#include "Osmosis/Server/Server.h"
#include "Osmosis/Server/Thread.h"
#include "Osmosis/HandleSignal.h"

namespace Osmosis {
namespace Server
{

Server::Server( boost::filesystem::path         rootPath,
	boost::asio::ip::tcp::endpoint  endpoint,
	ObjectStore::Store &            store,
	ObjectStore::Drafts &           drafts,
	ObjectStore::Labels &           labels,
	boost::asio::io_service & ioService ) :
	_rootPath( rootPath ),
	_store( store ),
	_drafts( drafts ),
	_labels( labels ),
	_acceptor( ioService, endpoint )
{
	boost::asio::socket_base::reuse_address option( true );
	_acceptor.set_option(option);
	HandleSignal::registerHandler( SIGUSR1, std::bind( &Server::flush, this ) );
	asyncAccept();
	TRACE_INFO( "Osmosis server up and running, waiting for connections" );
}

void Server::handleConnection( const boost::system::error_code& error )
{
	BACKTRACE_BEGIN
	_thread->run();
	asyncAccept();
	BACKTRACE_END
}

void Server::flush()
{
	_labels.flushLog();
}

void Server::asyncAccept()
{
	_thread = std::make_shared< Thread >( _rootPath, _store, _drafts, _labels );
	_acceptor.async_accept( _thread->socketForAccept(),
	                        boost::bind( &Server::handleConnection, this, boost::asio::placeholders::error ) );
}

} // namespace Server
} // namespace Osmosis
