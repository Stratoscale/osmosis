#ifndef __OSMOSIS_TCP_CONNECTION_H__
#define __OSMOSIS_TCP_CONNECTION_H__

#include <Osmosis/TCPSocket.h>
#include <boost/asio/ip/basic_resolver.hpp>

namespace Osmosis
{

class TCPConnection
{
public:
	TCPConnection( const std::string & hostname, unsigned short port );

	TCPSocket & socket();

	void connect();

private:
	boost::asio::io_service        _ioService;
	boost::asio::ip::tcp::socket   _socket;
	boost::asio::ip::tcp::endpoint _endpoint;
	TCPSocket                      _tcpSocket;

	void sendHandshake();

	void setTCPNoDelay();

	TCPConnection( const TCPConnection & rhs ) = delete;
	TCPConnection & operator= ( const TCPConnection & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_TCP_CONNECTION_H__
