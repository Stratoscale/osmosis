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
	CopyFile( const char * fromPath, const char * toPath );

	void copy();

private:
	ReadFile   _read;
	WriteFile  _write;

	void writeZeroCharacterAtLastOffset();

	CopyFile( const CopyFile & rhs ) = delete;
	CopyFile & operator= ( const CopyFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_COPY_FILE_H__
