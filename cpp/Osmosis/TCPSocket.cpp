#include "Osmosis/TCPSocket.h"
#include "Common/Debug.h"
#include "Common/Error.h"

namespace Osmosis
{

TCPSocket::TCPSocket( boost::asio::ip::tcp::socket & socket, unsigned int timeout ) :
    _socket( socket ),
    _deadline((boost::asio::io_context&)( _socket).get_executor().context()),
    _timeout( timeout )
{}

void TCPSocket::receiveAll( void * data, size_t length )
{
    boost::asio::io_service & ioService = (boost::asio::io_context&)( _socket).get_executor().context();
    while ( length > 0 ) {
        auto buffer = boost::asio::buffer( data, length );
        size_t received = 0;
        boost::system::error_code ec = boost::asio::error::would_block;
        ioService.reset();
        _deadline.expires_from_now( boost::posix_time::milliseconds( _timeout ) );
        _deadline.async_wait( boost::bind( &TCPSocket::checkDeadline, this, &ec ) );
        _socket.async_receive( buffer, boost::bind( &TCPSocket::handleIO,
                                                    this, _1, _2, &ec, &received ) );
        do {
            const auto nrHandlersRan = ioService.run_one();
            if ( 0 == nrHandlersRan )
                THROW( Error, "IO service got reset while receiving" );
        } while ( boost::asio::error::would_block == ec );
        _deadline.cancel();
        if ( ec ) {
            _socket.cancel();
            if ( boost::asio::error::misc_errors::eof == ec )
                throw boost::system::system_error( ec );
            else if ( boost::asio::error::timed_out == ec )
                THROW( Error, "Timeout while reading from " << _socket.remote_endpoint() );
            else
                THROW( Error, "An unexpected error occured while receiving: " << ec );
        }
        ASSERT( received <= length );
        length -= received;
        data = static_cast< unsigned char * >( data ) + received;
    }
}

void TCPSocket::sendAll( const void * data, size_t length )
{
    boost::asio::io_service & ioService = (boost::asio::io_context&)( _socket).get_executor().context();
    while ( length > 0 ) {
        auto buffer = boost::asio::buffer( data, length );
        size_t sent = 0;
        boost::system::error_code ec = boost::asio::error::would_block;
        ioService.reset();
        _deadline.expires_from_now( boost::posix_time::milliseconds( _timeout ) );
        _deadline.async_wait( boost::bind( &TCPSocket::checkDeadline, this, &ec ) );
        _socket.async_send( buffer,
                            boost::bind( &TCPSocket::handleIO,
                                        this, _1, _2, &ec, &sent
                            ) );
        do {
            const auto nrHandlersRan = ioService.run_one();
            if ( 0 == nrHandlersRan )
                THROW( Error, "IO service got reset while sending" );
        } while ( boost::asio::error::would_block == ec );
        _deadline.cancel();
        if ( ec ) {
            _socket.cancel();
            if ( boost::asio::error::misc_errors::eof == ec )
                throw boost::system::system_error( ec );
            else if ( boost::asio::error::timed_out == ec )
                THROW( Error, "Timeout while sending to " << _socket.remote_endpoint() );
            else
                THROW( Error, "An unexpected error occured while sending: " << ec );
        }
        ASSERT( sent <= length );
        length -= sent;
        data = static_cast< const unsigned char * >( data ) + sent;
    }
}

void TCPSocket::close() {
    _socket.close();
}

void TCPSocket::handleIO( const boost::system::error_code& ec, std::size_t size,
    boost::system::error_code * outEc, size_t * outSize )
{
    *outSize = size;
    *outEc = ec;
}

void TCPSocket::checkDeadline( boost::system::error_code *outEc )
{
    ASSERT( outEc != nullptr );
	if ( _deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now() ) {
        *outEc = boost::asio::error::timed_out;
	}
}

} // namespace Osmosis
