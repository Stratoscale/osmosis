#ifndef __OSMOSIS_STREAM_BUFFER_TO_SOCKET_H__
#define __OSMOSIS_STREAM_BUFFER_TO_SOCKET_H__

#include "Osmosis/Stream/Outgoing.h"

namespace Osmosis {
namespace Stream
{

class BufferToSocket
{
public:
	BufferToSocket( const void * buffer, size_t size, TCPSocket & socket );

	void transfer();

private:
	TCPSocket &            _socket;
	const unsigned char *  _buffer;
	size_t                 _left;
	Outgoing               _send;

	BufferToSocket( const BufferToSocket & rhs ) = delete;
	BufferToSocket & operator= ( const BufferToSocket & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_BUFFER_TO_SOCKET_H__
