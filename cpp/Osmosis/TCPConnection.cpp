#include <Osmosis/TCPConnection.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>
#include <Osmosis/Stream/AckOps.h>
#include "Common/Error.h"
#include <boost/asio/ip/basic_resolver.hpp>

namespace Osmosis
{

TCPConnection::TCPConnection( const std::string & hostname, unsigned short port, unsigned int tcpTimeout ):
	_socket( _ioService ),
	_tcpSocket( _socket, tcpTimeout ),
	_deadline( _ioService )
{
	boost::asio::ip::tcp::resolver resolver( _ioService );
	boost::asio::ip::tcp::resolver::query query( hostname, std::to_string( port ) );
	auto first = resolver.resolve( boost::asio::ip::tcp::resolver::query( boost::asio::ip::tcp::v4(), hostname, "0" ) );
	if ( first == boost::asio::ip::tcp::resolver::iterator() )
		THROW( Error, "Unable to resolve the hostname '" << hostname << "'" );
	boost::asio::ip::tcp::endpoint endpoint( first->endpoint().address(), port );
	boost::system::error_code ec = boost::asio::error::would_block;
	_socket.async_connect( endpoint, boost::bind( &TCPConnection::handleConnect, this, _1, &ec ) );
	_deadline.expires_from_now( boost::posix_time::milliseconds( tcpTimeout ) );
	_deadline.async_wait( boost::bind( &TCPConnection::checkConnectDeadline, this ) );
	do
	    _ioService.run_one();
	while ( ec == boost::asio::error::would_block );
	if ( ec or not _socket.is_open() ) {
	    THROW( Error, "Could not connect to " << hostname << ":" << port << ": " << ec.message() );
	}

	_deadline.cancel();
	_socket.non_blocking( true );
	ASSERT( _socket.is_open() );

	setTCPNoDelay();
	sendHandshake();
}

TCPSocket & TCPConnection::socket()
{
	return _tcpSocket;
}

void TCPConnection::sendHandshake()
{
	struct Tongue::Handshake handshake = {
		static_cast< unsigned >( Tongue::PROTOCOL_VERSION ),
		static_cast< unsigned >( Tongue::Compression::UNCOMPRESSED ) };
	_tcpSocket.sendAll( handshake );
	Stream::AckOps( _tcpSocket ).wait( "Handshake" );
}

void TCPConnection::setTCPNoDelay()
{
	boost::asio::ip::tcp::no_delay option( true );
	_socket.set_option(option);
}

void TCPConnection::handleConnect( const boost::system::error_code &ec,
								   boost::system::error_code * outEc )
{
	ASSERT( outEc != nullptr );
	*outEc = ec;
}

void TCPConnection::checkConnectDeadline()
{
	if ( _deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now() ) {
		_socket.cancel();
		_deadline.expires_at( boost::posix_time::pos_infin );
	}
}

} // namespace Osmosis
