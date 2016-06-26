#include <Osmosis/TCPConnection.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>
#include <Osmosis/Stream/AckOps.h>
#include "Common/Error.h"
#include <boost/asio/ip/basic_resolver.hpp>

namespace Osmosis
{

TCPConnection::TCPConnection( const std::string & hostname, unsigned short port ):
	_socket( _ioService ),
	_endpoint(),
	_tcpSocket( _socket )
{
	boost::asio::ip::tcp::resolver resolver( _ioService );
	boost::asio::ip::tcp::resolver::query query( hostname, std::to_string( port ) );
	auto first = resolver.resolve( boost::asio::ip::tcp::resolver::query( boost::asio::ip::tcp::v4(), hostname, "0" ) );
	if ( first == boost::asio::ip::tcp::resolver::iterator() )
		THROW( Error, "Unable to resolve the hostname '" << hostname << "'" );
	boost::asio::ip::tcp::endpoint endpoint( first->endpoint().address(), port );
	_endpoint = endpoint;
	connect();
}

TCPSocket & TCPConnection::socket()
{
	return _tcpSocket;
}

void TCPConnection::connect()
{
	if ( _socket.is_open() ) {
		_socket.close();
	}
	_socket.connect( _endpoint );
	ASSERT( _socket.is_open() );
	setTCPNoDelay();
	sendHandshake();
}

void TCPConnection::sendHandshake()
{
	struct Tongue::Handshake handshake = {
		static_cast< unsigned >( Tongue::MIN_SUPPORTED_PROTOCOL_VERSION ),
		static_cast< unsigned >( Tongue::Compression::UNCOMPRESSED ) };
	_tcpSocket.sendAll( handshake );
	Stream::AckOps( _tcpSocket ).wait( "Handshake" );
}

void TCPConnection::setTCPNoDelay()
{
	boost::asio::ip::tcp::no_delay option( true );
	_socket.set_option(option);
}

} // namespace Osmosis
