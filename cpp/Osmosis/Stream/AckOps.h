#ifndef __OSMOSIS_STREAM_ACK_OPS_H__
#define __OSMOSIS_STREAM_ACK_OPS_H__

namespace Osmosis {
namespace Stream
{

class AckOps
{
public:
	AckOps( TCPSocket & socket ):
		_socket( socket )
	{}

	void wait( const char * waitReason )
	{
		try {
			struct Tongue::Header header = _socket.receiveAll< struct Tongue::Header >();
			if ( header.opcode != static_cast< unsigned >( Tongue::Opcode::ACK ) )
				THROW( Error, "Peer did not ack, found byte " << header.opcode << ", when " << waitReason );
		} catch ( boost::system::system_error & e ) {
			if ( e.code() == boost::asio::error::eof )
				THROW( Error, "Peer terminated connection instead of sending ack, when " << waitReason );
			throw;
		}
	}

	void sendAck()
	{
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::ACK ) };
		_socket.sendAll( header );
	}

private:
	TCPSocket &            _socket;

	AckOps( const AckOps & rhs ) = delete;
	AckOps & operator= ( const AckOps & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_ACK_OPS_H__
