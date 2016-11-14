#ifndef __OSMOSIS_TCPSOCKET_H__
#define __OSMOSIS_TCPSOCKET_H__

#include <boost/asio.hpp>
#include "Common/Debug.h"

namespace Osmosis
{

class TCPSocket
{
public:
	TCPSocket( boost::asio::ip::tcp::socket & socket ) :
		_socket( socket )
	{}

	void receiveAll( void * data, size_t length )
	{
		while ( length > 0 ) {
			auto buffer = boost::asio::buffer( data, length );
			size_t received = _socket.receive( buffer );
			ASSERT( received <= length );
			length -= received;
			data = static_cast< unsigned char * >( data ) + received;
		}
	}

	template < class Struct >
	Struct receiveAll()
	{
		Struct result;
		receiveAll( & result, sizeof( result ) );
		return result;
	}

	void sendAll( const void * data, size_t length )
	{
		while ( length > 0 ) {
			auto buffer = boost::asio::buffer( data, length );
			size_t sent = _socket.send( buffer );
			ASSERT( sent <= length );
			length -= sent;
			data = static_cast< const unsigned char * >( data ) + sent;
		}
	}

	template < class Struct >
	void sendAll( const Struct & data )
	{
		sendAll( & data, sizeof( data ) );
	}

	template < class Struct1, class Struct2 >
	void sendAllConcated( const Struct1 & data1, const Struct2 & data2 )
	{
		unsigned char buffer[ sizeof( data1 ) + sizeof( data2 ) ];
		memcpy( buffer, & data1, sizeof( data1 ) );
		memcpy( buffer + sizeof( data1 ), & data2, sizeof( data2 ) );
		sendAll( buffer, sizeof( buffer ) );
	}

	void close() {
		_socket.close();
	}

private:
	boost::asio::ip::tcp::socket & _socket;

	TCPSocket( const TCPSocket & rhs ) = delete;
	TCPSocket & operator= ( const TCPSocket & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_TCPSOCKET_H__
