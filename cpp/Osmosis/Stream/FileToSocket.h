#ifndef __OSMOSIS_STREAM_FILE_TO_SOCKET_H__
#define __OSMOSIS_STREAM_FILE_TO_SOCKET_H__

#include "Osmosis/Stream/ReadFile.h"
#include "Osmosis/Stream/Outgoing.h"
#include "Osmosis/Stream/WaitForAck.h"

namespace Osmosis {
namespace Stream
{

class FileToSocket
{
public:
	FileToSocket( const char * filename, TCPSocket & socket ) :
		_socket( socket ),
		_read( filename ),
		_send( socket ),
		_waitForAck( socket )
	{}

	void transfer()
	{
		bool lastReadWasHole = false;
		while ( not _read.done() ) {
			lastReadWasHole = _read.guessHole();
			if ( not lastReadWasHole ) 
				_send.send( _read.offset(), _read.buffer(), _read.length() );
			_read.next();
		}
		if ( lastReadWasHole )
			sendZeroCharacterAtLastOffset();
		_send.sendEOF();
		_waitForAck.wait( "sent file" );
	}

private:
	TCPSocket &  _socket;
	ReadFile     _read;
	Outgoing     _send;
	WaitForAck   _waitForAck; 

	void sendZeroCharacterAtLastOffset()
	{
		unsigned char zero = 0;
		ASSERT( _read.totalLength() >= 1 );
		_send.send( _read.totalLength() - sizeof( zero ), & zero, sizeof( zero ) );
	}

	FileToSocket( const FileToSocket & rhs ) = delete;
	FileToSocket & operator= ( const FileToSocket & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_FILE_TO_SOCKET_H__
