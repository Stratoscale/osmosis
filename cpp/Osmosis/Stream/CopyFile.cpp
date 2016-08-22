#include "Osmosis/Stream/CopyFile.h"
#include "Osmosis/Stream/ReadFile.h"
#include "Osmosis/Stream/WriteFile.h"

namespace Osmosis {
namespace Stream
{

CopyFile::CopyFile( const char * fromPath, const char * toPath ) :
	_read( fromPath ),
	_write( toPath )
{}

void CopyFile::copy()
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

void CopyFile::writeZeroCharacterAtLastOffset()
{
	unsigned char zero = 0;
	ASSERT( _read.totalLength() >= 1 );
	_write.write( _read.totalLength() - sizeof( zero ), & zero, sizeof( zero ) );
}

} // namespace Stream
} // namespace Osmosis
