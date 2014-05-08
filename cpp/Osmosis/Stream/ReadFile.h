#ifndef __OSMOSIS_STREAM_READ_FILE_H__
#define __OSMOSIS_STREAM_READ_FILE_H__

#include "Osmosis/FileDescriptor.h"

namespace Osmosis {
namespace Stream
{

class ReadFile
{
public:
	ReadFile( const char * filename ) :
		_filename( filename ),
		_descriptor( FileDescriptor::openForRead( filename ) ),
		_offset( 0 ),
		_bytesInBuffer( 0 )
	{
		next();
	}

	void next()
	{
		_offset += _bytesInBuffer;
		ssize_t bytesRead = read( _descriptor.fd(), _buffer, sizeof( _buffer ) );
		if ( bytesRead < 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to read from file '" << _filename <<
					"' at offset " << _offset );
		ASSERT( static_cast< size_t >( bytesRead ) <= sizeof( _buffer ) );
		_bytesInBuffer = bytesRead;
	}

	bool done() const
	{
		return _bytesInBuffer == 0;
	}

	size_t offset() const
	{
		ASSERT( ! done() );
		return _offset;
	}

	unsigned length() const
	{
		ASSERT( ! done() );
		return _bytesInBuffer;
	}

	const unsigned char * buffer() const
	{
		ASSERT( ! done() );
		return _buffer;
	}

	size_t totalLength() const
	{
		ASSERT( done() );
		return _offset;
	}

	bool guessHole() const
	{
		ASSERT( ! done() );
		static unsigned char zero[ sizeof( _buffer ) ] = { 0 };
		if ( _bytesInBuffer != sizeof( _buffer ) )
			return false;
		return memcmp( _buffer, zero, sizeof( _buffer ) ) == 0;
	}

private:
	const std::string _filename;
	FileDescriptor _descriptor;
	size_t _offset;
	unsigned char _buffer[ 4096 ];
	unsigned _bytesInBuffer;

	ReadFile( const ReadFile & rhs ) = delete;
	ReadFile & operator= ( const ReadFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_READ_FILE_H__
