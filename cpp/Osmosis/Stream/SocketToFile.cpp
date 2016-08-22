#include "Osmosis/Stream/SocketToFile.h"

namespace Osmosis {
namespace Stream
{

SocketToFile::SocketToFile( TCPSocket & socket, const char * filename ) :
	_socket( socket ),
	_incoming( socket ),
	_write( filename )
{}

void SocketToFile::transfer()
{
	while ( not _incoming.done() ) {
		_write.write( _incoming.offset(), _incoming.buffer(), _incoming.bufferLength() );
		_incoming.next();
	}
}

} // namespace Stream
} // namespace Osmosis
