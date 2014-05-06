#ifndef __OSMOSIS_CLIENT_CONNECT_H__
#define __OSMOSIS_CLIENT_CONNECT_H__

namespace Osmosis {
namespace Client
{

class Connect
{
public:
	Connect( const std::string & hostname, unsigned short port ) :
		_socket( _ioService ),
		_tcpSocket( _socket )
	{
		boost::asio::ip::tcp::resolver resolver( _ioService );
		boost::asio::ip::tcp::resolver::query query( hostname, std::to_string( port ) );
		auto first = resolver.resolve( query );
		if ( first == boost::asio::ip::tcp::resolver::iterator() )
			THROW( Error, "Unable to resolve the hostname '" << hostname << "'" );
		_socket.connect( * first );
		ASSERT( _socket.is_open() );
		TRACE_INFO( "Connected to " << _socket.remote_endpoint() );
		boost::asio::ip::tcp::no_delay option( true );
		_socket.set_option(option);

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

	Connect( const Connect & rhs ) = delete;
	Connect & operator= ( const Connect & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CONNECT_H__
