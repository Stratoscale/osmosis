#ifndef __OSMOSIS_STREAM_ACK_OPS_H__
#define __OSMOSIS_STREAM_ACK_OPS_H__

#include <Common/Error.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Stream/AckOps.h>

namespace Osmosis {
namespace Stream
{

class AckOps
{
public:
	AckOps( TCPSocket & socket );

	void wait( const char * waitReason );

	void sendAck();

private:
	TCPSocket &            _socket;

	AckOps( const AckOps & rhs ) = delete;
	AckOps & operator= ( const AckOps & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_ACK_OPS_H__
