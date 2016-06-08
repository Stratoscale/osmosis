#include <string>
#include <Osmosis/TCPConnection.h>
#include <Osmosis/Stream/AckOps.h>

namespace Osmosis
{
TCPConnection::TCPConnection( const std::string & hostname, unsigned short port ):
	_socket( _ioService ),
	_endpoint(),
	_tcpSocket( _socket )
{
	std::cout << "Nothing happens here";
}

TCPSocket & TCPConnection::socket()
{
	return _tcpSocket;
}

void TCPConnection::connect()
{
}

void TCPConnection::sendHandshake()
{
	Stream::AckOps( _tcpSocket ).wait( "Handshake" );
}

void TCPConnection::setTCPNoDelay()
{
}
}
