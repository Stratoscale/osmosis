#include "Osmosis/Stream/BufferToSocket.h"
#include "Osmosis/Stream/Outgoing.h"

namespace Osmosis {
namespace Stream
{

BufferToSocket::BufferToSocket( const void * buffer, size_t size, TCPSocket & socket ) :
	_socket( socket ),
	_buffer( static_cast< const unsigned char * >( buffer ) ),
	_left( size ),
	_send( socket )
{}

void BufferToSocket::transfer()
{
	size_t offset = 0;
	while ( _left > 0 ) {
		size_t chunkLength = std::min( _left, 4096UL );
		_send.send( offset, _buffer, chunkLength );
		_buffer += chunkLength;
		_left -= chunkLength;
		offset += chunkLength;
	}
	_send.sendEOF();
}

} // namespace Stream
} // namespace Osmosis
