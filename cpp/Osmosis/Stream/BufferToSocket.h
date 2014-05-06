#ifndef __OSMOSIS_STREAM_BUFFER_TO_SOCKET_H__
#define __OSMOSIS_STREAM_BUFFER_TO_SOCKET_H__

#include "Osmosis/Stream/Outgoing.h"

namespace Osmosis {
namespace Stream
{

class BufferToSocket
{
public:
	BufferToSocket( const void * buffer, size_t size, TCPSocket & socket ) :
		_socket( socket ),
		_buffer( static_cast< const unsigned char * >( buffer ) ),
		_left( size ),
		_send( socket )
	{}

	void transfer()
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
