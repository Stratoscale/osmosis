#ifndef __OSMOSIS_TCP_CONNECTION_H__
#define __OSMOSIS_TCP_CONNECTION_H__

namespace Osmosis
{

class TCPConnection
{
public:
	TCPConnection( const std::string & hostname, unsigned short port ):
		_socket( _ioService ),
		_tcpSocket( _socket )
	{
		boost::asio::ip::tcp::resolver resolver( _ioService );
		boost::asio::ip::tcp::resolver::query query( hostname, std::to_string( port ) );
		auto first = resolver.resolve( boost::asio::ip::tcp::resolver::query( boost::asio::ip::tcp::v4(), hostname, "0" ) );
		if ( first == boost::asio::ip::tcp::resolver::iterator() )
			THROW( Error, "Unable to resolve the hostname '" << hostname << "'" );
		boost::asio::ip::tcp::endpoint endpoint( first->endpoint().address(), port );
		_socket.connect( endpoint );
		ASSERT( _socket.is_open() );

		setTCPNoDelay();
		sendHandshake();
	}

	TCPSocket & socket()
	{
		return _tcpSocket;
	}

private:
	boost::asio::io_service       _ioService;
	boost::asio::ip::tcp::socket  _socket;
	TCPSocket                     _tcpSocket; 

	void sendHandshake()
	{
		struct Tongue::Handshake handshake = {
			static_cast< unsigned >( Tongue::PROTOCOL_VERSION ),
			static_cast< unsigned >( Tongue::Compression::UNCOMPRESSED ) };
		_tcpSocket.sendAll( handshake );
		Stream::AckOps( _tcpSocket ).wait( "Handshake" );
	}

	void setTCPNoDelay()
	{
		boost::asio::ip::tcp::no_delay option( true );
		_socket.set_option(option);
	}

	TCPConnection( const TCPConnection & rhs ) = delete;
	TCPConnection & operator= ( const TCPConnection & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_TCP_CONNECTION_H__
