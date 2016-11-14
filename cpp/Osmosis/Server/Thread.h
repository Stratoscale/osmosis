#ifndef __OSMOSIS_SERVER_THREAD_H__
#define __OSMOSIS_SERVER_THREAD_H__

#include <boost/filesystem.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/ObjectStore/Store.h>
#include <Osmosis/ObjectStore/Labels.h>
#include <Osmosis/ObjectStore/Drafts.h>
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace Server
{

class Thread : public std::enable_shared_from_this< Thread >
{
public:
	Thread( boost::filesystem::path  rootPath,
		ObjectStore::Store &     store,
		ObjectStore::Drafts &    drafts,
		ObjectStore::Labels &    labels );

	boost::asio::ip::tcp::socket & socketForAccept();

	void run();

private:
	boost::filesystem::path       _rootPath;
	ObjectStore::Store &          _store;
	ObjectStore::Drafts &         _drafts;
	ObjectStore::Labels &         _labels;
	boost::asio::io_service       _ioService;
	boost::asio::ip::tcp::socket  _boostSocket;
	TCPSocket                     _socket; 
	bool                          _tcpNoDelay;

	void thread();

	bool work();

	void handshake();

	void setTCPNoDelay( bool value );

	void setTCPKeepalive( bool value );

	Thread( const Thread & rhs ) = delete;
	Thread & operator= ( const Thread & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_THREAD_H__
