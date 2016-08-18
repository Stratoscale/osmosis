#ifndef __OSMOSIS_STREAM_INCOMING_H__
#define __OSMOSIS_STREAM_INCOMING_H__

#include "Osmosis/TCPSocket.h"
#include "Osmosis/Tongue.h"

namespace Osmosis {
namespace Stream
{

class Incoming
{
public:
	Incoming( TCPSocket & socket ) :
		_socket( socket )
	{
		next();
	}

	void next()
	{
		struct Tongue::Chunk chunk = _socket.receiveAll< struct Tongue::Chunk >();
		_bytesInBuffer = chunk.bytes;
		_offset = chunk.offset;
		if ( _bytesInBuffer == 0 )
			return;
		_socket.receiveAll( _buffer, _bytesInBuffer );
	}

	bool done() const
	{
		return _bytesInBuffer == 0;
	}

	size_t offset() const
	{
		ASSERT( ! done() );
		return _offset;
	}

	const unsigned char * buffer() const
	{
		ASSERT( ! done() );
		return _buffer;
	}

	unsigned bufferLength() const
	{
		ASSERT( ! done() );
		return _bytesInBuffer;
	}

private:
	TCPSocket & _socket;
	unsigned char _buffer[ 4096 ];
	unsigned _bytesInBuffer;
	size_t _offset;

	Incoming( const Incoming & rhs ) = delete;
	Incoming & operator= ( const Incoming & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_INCOMING_H__
