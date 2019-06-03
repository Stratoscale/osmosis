#ifndef __OSMOSIS_SERVER_SERVER_H__
#define __OSMOSIS_SERVER_SERVER_H__

#include <boost/asio.hpp>
#include "Osmosis/Server/Thread.h"
#include "Osmosis/HandleSignal.h"

namespace Osmosis {
namespace Server
{

class Server
{
public:
	Server( boost::filesystem::path         rootPath,
		boost::asio::ip::tcp::endpoint  endpoint,
		boost::asio::io_service &       ioService );

	void run();

private:
	boost::filesystem::path         _rootPath;
	ObjectStore::Store              _store;
	ObjectStore::Drafts             _drafts;
	ObjectStore::Labels             _labels;
	boost::asio::ip::tcp::acceptor  _acceptor;
	std::shared_ptr< Thread >       _thread;


	void flush();

	void asyncAccept();

	void handleConnection( const boost::system::error_code& error );

	Server( const Server & rhs ) = delete;
	Server & operator= ( const Server & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_SERVER_H__
