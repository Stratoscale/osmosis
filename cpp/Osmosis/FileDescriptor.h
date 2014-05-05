#ifndef __OSMOSIS_FILE_DESCRIPTOR_H__
#define __OSMOSIS_FILE_DESCRIPTOR_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Common/Debug.h"
#include <boost/filesystem.hpp>

namespace Osmosis
{

class FileDescriptor
{
public:
	FileDescriptor( FileDescriptor && other ) :
		_fd( other._fd )
	{
		other._fd = -1;
	}

	static FileDescriptor openForRead( const char * filename )
	{
		int fd = open( filename, O_RDONLY );
		if ( fd < 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to open file '" << filename << "' for reading" );
		return std::move( FileDescriptor( fd ) );
	}

	static FileDescriptor openCreateWrite( const char * filename )
	{
		int fd = open( filename, O_WRONLY | O_CREAT | O_EXCL, 0644 );
		if ( fd < 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to create file '" << filename << "' for writing" );
		return std::move( FileDescriptor( fd ) );
	}

	~FileDescriptor()
	{
		if ( _fd < 0 )
			return;
		int result = close( _fd );
		if ( result != 0 )
			TRACE_ERROR( "File descriptor close failed: " << result << ". Ignoring" );
	}

	int fd() const
	{
		return _fd;
	}

private:
	int _fd;

	FileDescriptor( int fd ) :
		_fd( fd )
	{
		ASSERT( _fd >= 0 );
	}

	FileDescriptor( const FileDescriptor & rhs ) = delete;
	FileDescriptor & operator= ( const FileDescriptor & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_FILE_DESCRIPTOR_H__
