#ifndef __OSMOSIS_STREAM_WRITE_FILE_H__
#define __OSMOSIS_STREAM_WRITE_FILE_H__

#include <cstddef>
#include <string>
#include "Osmosis/FileDescriptor.h"

namespace Osmosis {
namespace Stream
{

class WriteFile
{
public:
	WriteFile( const char * filename );

	void write( size_t offset, const void * data, const unsigned length );

private:
	const std::string _filename;
	FileDescriptor _descriptor;
	size_t _offset;

	void seek( size_t offset );

	WriteFile( const WriteFile & rhs ) = delete;
	WriteFile & operator= ( const WriteFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_WRITE_FILE_H__
