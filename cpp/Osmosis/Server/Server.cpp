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
	ObjectStore::Labels &           labels ) :
	_rootPath( rootPath ),
	_store( store ),
	_drafts( drafts ),
	_labels( labels ),
	_acceptor( _ioService, endpoint )
{
	boost::asio::socket_base::reuse_address option( true );
	_acceptor.set_option(option);
	HandleSignal::registerHandler( SIGUSR1, std::bind( &Server::flush, this ) );
}

void Server::run()
{
	TRACE_INFO( "Osmosis server up and running, waiting for connections" );
	BACKTRACE_BEGIN
	while ( true ) {
		std::shared_ptr< Thread > thread = std::make_shared< Thread >( _rootPath, _store, _drafts, _labels );
		_acceptor.accept( thread->socketForAccept() );
		thread->run();
	}
	BACKTRACE_END
}

void Server::flush()
{
	_labels.flushLog();
}

} // namespace Server
} // namespace Osmosis
