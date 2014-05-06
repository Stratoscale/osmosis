#ifndef __OSMOSIS_STREAM_SOCKET_TO_BUFFER_H__
#define __OSMOSIS_STREAM_SOCKET_TO_BUFFER_H__

#include "Osmosis/Stream/Incoming.h"

namespace Osmosis {
namespace Stream
{

class SocketToBuffer
{
public:
	SocketToBuffer( TCPSocket & socket ):
		_socket( socket ),
		_incoming( socket ),
		_offset( 0 )
	{}

	void transfer()
	{
		while ( not _incoming.done() ) {
			handleOffsetGaps();
			_buffer.write( reinterpret_cast< const char * >( _incoming.buffer() ),
					_incoming.bufferLength() );
			_incoming.next();
		}
	}

	std::string data()
	{
		return std::move( _buffer.str() );
	}

private:
	TCPSocket &         _socket;
	Incoming            _incoming;
	std::ostringstream  _buffer;
	size_t              _offset; 

	void handleOffsetGaps()
	{
		if ( _incoming.offset() != _offset ) {
			if ( _incoming.offset() < _offset )
				THROW( Error, "Incoming offset goes backwards, not implemented" );
			for ( ; _offset < _incoming.offset(); ++ _offset )
				_buffer << '\0';
		}
	}

	SocketToBuffer( const SocketToBuffer & rhs ) = delete;
	SocketToBuffer & operator= ( const SocketToBuffer & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_SOCKET_TO_BUFFER_H__
