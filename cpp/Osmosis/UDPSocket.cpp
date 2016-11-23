#include <boost/bind.hpp>
#include "Osmosis/UDPSocket.h"
#include "Osmosis/Debug.h"
#include "Common/Error.h"

namespace Osmosis {

UDPSocket::UDPSocket( unsigned short listeningPort, bool bindAsLocalhost ) :
    _socket( _ioService, boost::asio::ip::udp::endpoint(
    (bindAsLocalhost ? boost::asio::ip::address::from_string( "127.0.0.1" ) : boost::asio::ip::address_v4()),
    listeningPort ) ),
_deadline( _ioService )
{
    _deadline.expires_at( boost::posix_time::pos_infin );
    _socket.set_option( boost::asio::socket_base::reuse_address( true ) );
    _socket.set_option( boost::asio::socket_base::broadcast( true ) );
    checkDeadline();
}

std::size_t UDPSocket::receive( unsigned char * buffer, size_t bufferSize,
	boost::asio::ip::udp::endpoint & remoteEndpoint,
	int timeoutInMilliseconds, boost::system::error_code & ec )
{
    _deadline.expires_from_now( boost::posix_time::milliseconds( timeoutInMilliseconds ) );
    ec = boost::asio::error::would_block;
    std::size_t length = 0;
    _socket.async_receive_from( boost::asio::buffer( buffer, bufferSize ), remoteEndpoint,
        boost::bind( &UDPSocket::handleReceive, this, _1, _2, &ec, &length ) );
    do
        _ioService.run_one();
    while ( ec == boost::asio::error::would_block );
    return length;
}

void UDPSocket::checkDeadline()
{
    if ( _deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now() ) {
        _socket.cancel();
        _deadline.expires_at( boost::posix_time::pos_infin );
    } else
        _deadline.async_wait( boost::bind( &UDPSocket::checkDeadline, this ) );
}

void UDPSocket::handleReceive( const boost::system::error_code ec, std::size_t length,
                               boost::system::error_code * out_ec, std::size_t * outLength)
{
    *out_ec = ec;
    *outLength = length;
}

void UDPSocket::handleSend( const boost::system::error_code ec, std::size_t nrBytesTransferred )
{
    if (ec) {
        THROW( Error, "Could not broadcast a UDP packet: " << ec.message() );
    }
}

void UDPSocket::broadcast( const unsigned char * data, size_t size, unsigned short port )
{
    boost::asio::ip::udp::endpoint receiverEndpoint( boost::asio::ip::address_v4::broadcast(), port );
    _socket.async_send_to( boost::asio::buffer( data, size ), receiverEndpoint,
	                       boost::bind( &UDPSocket::handleSend, this,
	                                    boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred ) );
	_ioService.run_one();
}

} // namespace Osmosis
