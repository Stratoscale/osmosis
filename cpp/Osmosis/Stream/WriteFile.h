#ifndef __OSMOSIS_STREAM_WRITE_FILE_H__
#define __OSMOSIS_STREAM_WRITE_FILE_H__

namespace Osmosis {
namespace Stream
{

class WriteFile
{
public:
	WriteFile( const char * filename ) :
		_filename( filename ),
		_descriptor( FileDescriptor::openCreateWrite( filename ) ),
		_offset( 0 )
	{}

	void write( size_t offset, const void * data, unsigned length )
	{
		ASSERT( data != nullptr );
		if ( length == 0 )
			return;
		if ( offset != _offset )
			seek( offset );

		ssize_t written = ::write( _descriptor.fd(), data, length );
		if ( written < 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to write to '" << _filename <<
				"' at offset " << _offset );
		if ( written != length )
			THROW( Error, "linux write system call return partial data was written: " <<
					"expected: " << length << " but only " << written << " was written" );
		_offset += length;
	}

private:
	const std::string _filename;
	FileDescriptor _descriptor;
	size_t _offset;

	void seek( size_t offset )
	{
		ASSERT( offset != _offset );
		off_t result = lseek( _descriptor.fd(), offset, SEEK_SET );
		if ( result < 0 )
			THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to seek in '" <<
					_filename << "' for writing at offset " << offset );
		ASSERT( static_cast< size_t >( result ) == offset );
		_offset = offset;
	}

	WriteFile( const WriteFile & rhs ) = delete;
	WriteFile & operator= ( const WriteFile & rhs ) = delete;
};

} // namespace Stream
} // namespace Osmosis

#endif // __OSMOSIS_STREAM_WRITE_FILE_H__
