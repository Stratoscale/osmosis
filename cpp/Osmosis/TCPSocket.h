#ifndef __OSMOSIS_TCPSOCKET_H__
#define __OSMOSIS_TCPSOCKET_H__

#include <boost/asio.hpp>

namespace Osmosis
{

class TCPSocket
{
public:
	TCPSocket( boost::asio::ip::tcp::socket & socket, unsigned int timeout );

	void receiveAll( void * data, size_t length );

	template < class Struct >
	Struct receiveAll()
	{
		Struct result;
		receiveAll( & result, sizeof( result ) );
		return result;
	}

	void sendAll( const void * data, size_t length );

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

	void close();

private:
	boost::asio::ip::tcp::socket & _socket;
	boost::asio::deadline_timer    _deadline;
	unsigned int _timeout;

	void handleIO( const boost::system::error_code& ec, std::size_t size,
	               boost::system::error_code * outEc, size_t * outSize );

	void checkDeadline( boost::system::error_code *outEc );

	TCPSocket( const TCPSocket & rhs ) = delete;
	TCPSocket & operator= ( const TCPSocket & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_TCPSOCKET_H__
