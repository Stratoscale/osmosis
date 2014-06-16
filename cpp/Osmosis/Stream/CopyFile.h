#ifndef __OSMOSIS_STREAM_COPY_FILE_H__
#define __OSMOSIS_STREAM_COPY_FILE_H__

#include "Osmosis/Stream/ReadFile.h"
#include "Osmosis/Stream/WriteFile.h"

namespace Osmosis {
namespace Stream
{

class CopyFile
{
public:
	CopyFile( const char * fromPath, const char * toPath ) :
		_read( fromPath ),
		_write( toPath )
	{}

	void copy()
	{
		bool lastReadWasHole = false;
		while ( not _read.done() ) {
			lastReadWasHole = _read.guessHole();
			if ( not lastReadWasHole )
				_write.write( _read.offset(), _read.buffer(), _read.length() );
			_read.next();
		}
		if ( lastReadWasHole )
			writeZeroCharacterAtLastOffset();
	}

private:
	ReadFile   _read;
	WriteFile  _write;

	void writeZeroCharacterAtLastOffset()
	{
		unsigned char zero = 0;
		ASSERT( _read.totalLength() >= 1 );
		_write.write( _read.totalLength() - sizeof( zero ), & zero, sizeof( zero ) );
	}

	CopyFile( const CopyFile & rhs ) = delete;
	CopyFile & operator= ( const CopyFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_COPY_FILE_H__
