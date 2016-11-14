#include "Osmosis/Stream/WriteFile.h"
#include "Osmosis/FileDescriptor.h"
#include "Common/Debug.h"
#include "Common/Error.h"

namespace Osmosis {
namespace Stream
{

WriteFile::WriteFile( const char * filename ) :
	_filename( filename ),
	_descriptor( FileDescriptor::openCreateWrite( filename ) ),
	_offset( 0 )
{}

void WriteFile::write( size_t offset, const void * data, const unsigned length )
{
	ASSERT( data != nullptr );
	if ( length == 0 )
		return;
	if ( offset != _offset )
		seek( offset );

	auto nrBytesLeftToWrite = length;
	unsigned char *datap = (unsigned char *) data;
	while ( nrBytesLeftToWrite > 0 ) {
		ssize_t written = ::write( _descriptor.fd(), datap, nrBytesLeftToWrite );
		if ( written <= 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to write to '" << _filename <<
				"' at offset " << _offset );
		if ( written != nrBytesLeftToWrite )
			TRACE_WARNING("While writing to file '" << _filename << "': linux 'write' system call return "
				"value indicates that partial data was written: " <<
				"expected: " << nrBytesLeftToWrite << " but only " << written << " was written. " <<
				nrBytesLeftToWrite << "bytes left to write." );
		_offset += written;
		nrBytesLeftToWrite -= written;
		datap += written;
	}
}


void WriteFile::seek( size_t offset )
{
	ASSERT( offset != _offset );
	off_t result = lseek( _descriptor.fd(), offset, SEEK_SET );
	if ( result < 0 )
		THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to seek in '" <<
				_filename << "' for writing at offset " << offset );
	ASSERT( static_cast< size_t >( result ) == offset );
	_offset = offset;
}

} // namespace Stream
} // namespace Osmosis
