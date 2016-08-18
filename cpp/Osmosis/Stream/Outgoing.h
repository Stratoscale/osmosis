#ifndef __OSMOSIS_STREAM_OUTGOING_H__
#define __OSMOSIS_STREAM_OUTGOING_H__

#include "Osmosis/TCPSocket.h"

namespace Osmosis {
namespace Stream
{

class Outgoing
{
public:
	Outgoing( TCPSocket & socket ) :
		_socket( socket )
	{}

	void send( size_t offset, const void * data, unsigned length )
	{
		ASSERT( length > 0 );
		struct Tongue::Chunk chunk;
		chunk.offset = offset;
		chunk.bytes = length;
		_socket.sendAll( chunk );
		_socket.sendAll( data, length );
	}

	void sendEOF()
	{
		struct Tongue::Chunk chunk;
		chunk.offset = 0;
		chunk.bytes = 0;
		_socket.sendAll( chunk );
	}

private:
	TCPSocket & _socket;

	Outgoing( const Outgoing & rhs ) = delete;
	Outgoing & operator= ( const Outgoing & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_OUTGOING_H__
