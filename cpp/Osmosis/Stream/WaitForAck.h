#ifndef __OSMOSIS_STREAM_WAIT_FOR_ACK_H__
#define __OSMOSIS_STREAM_WAIT_FOR_ACK_H__

namespace Osmosis {
namespace Stream
{

class WaitForAck
{
public:
	WaitForAck( TCPSocket & socket ):
		_socket( socket )
	{}

	void wait( const char * waitReason )
	{
		try {
			struct Tongue::Header header = _socket.recieveAll< struct Tongue::Header >();
			if ( header.opcode != static_cast< unsigned >( Tongue::Opcode::ACK ) )
				THROW( Error, "Peer did not ack, found byte " << header.opcode << ", when " << waitReason );
		} catch ( boost::system::system_error & e ) {
			if ( e.code() == boost::asio::error::eof )
				THROW( Error, "Peer terminated connection instead of sending ack, when " << waitReason );
			throw;
		}
	}

private:
	TCPSocket &            _socket;

	WaitForAck( const WaitForAck & rhs ) = delete;
	WaitForAck & operator= ( const WaitForAck & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_WAIT_FOR_ACK_H__
