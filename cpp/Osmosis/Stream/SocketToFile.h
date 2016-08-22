#ifndef __OSMOSIS_STREAM_SOCKET_TO_FILE_H__
#define __OSMOSIS_STREAM_SOCKET_TO_FILE_H__

#include "Osmosis/Stream/Incoming.h"
#include "Osmosis/Stream/WriteFile.h"

namespace Osmosis {
namespace Stream
{

class SocketToFile
{
public:
	SocketToFile( TCPSocket & socket, const char * filename );

	void transfer();

private:
	TCPSocket & _socket;
	Incoming _incoming;
	WriteFile _write;

	SocketToFile( const SocketToFile & rhs ) = delete;
	SocketToFile & operator= ( const SocketToFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_SOCKET_TO_FILE_H__
