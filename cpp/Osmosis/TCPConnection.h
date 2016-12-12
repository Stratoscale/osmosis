#ifndef __OSMOSIS_TCP_CONNECTION_H__
#define __OSMOSIS_TCP_CONNECTION_H__

#include <Osmosis/TCPSocket.h>
#include <boost/asio/ip/basic_resolver.hpp>

namespace Osmosis
{

class TCPConnection
{
public:
	TCPConnection( const std::string & hostname, unsigned short port, unsigned int tcpTimeout );

	TCPSocket & socket();

private:
	boost::asio::io_service        _ioService;
	boost::asio::ip::tcp::socket   _socket;
	TCPSocket                      _tcpSocket;
	boost::asio::deadline_timer    _deadline;

	void sendHandshake();

	void setTCPNoDelay();

	void handleConnect( const boost::system::error_code &ec, boost::system::error_code * outEc );

	void checkConnectDeadline();

	TCPConnection( const TCPConnection & rhs ) = delete;
	TCPConnection & operator= ( const TCPConnection & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_TCP_CONNECTION_H__
