#include <boost/system/system_error.hpp>
#include <boost/asio/error.hpp>
#include <Osmosis/Stream/AckOps.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>
#include <Common/Error.h>

namespace Osmosis {
namespace Stream
{

AckOps::AckOps( TCPSocket & socket ):
	_socket( socket )
{}

void AckOps::wait( const char * waitReason )
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

void AckOps::sendAck()
{
	struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::ACK ) };
	_socket.sendAll( header );
}

} // namespace Stream
} // namespace Osmosis
