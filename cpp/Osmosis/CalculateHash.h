#ifndef __OSMOSIS_CALCULATE_HASH_H__
#define __OSMOSIS_CALCULATE_HASH_H__

#include "Osmosis/FileDescriptor.h"
#include "Osmosis/Hash.h"

namespace Osmosis
{

class CalculateHash
{
public:
	static Hash MD5( const void * buffer, unsigned length )
	{
		static Tongue::Hash raw;
		MD5_CTX context;
		int result = MD5_Init( & context );
		if ( result != 1 )
			THROW( Error, "Unable to init MD5 calculation" );
		result = MD5_Update( & context, buffer, length );
		if ( result != 1 )
			THROW( Error, "Unable to continue MD5 calculation" );
		ASSERT( sizeof( raw.hash ) >= MD5_DIGEST_LENGTH );
		result = MD5_Final( raw.hash, & context );
		if ( result != 1 )
			THROW( Error, "Unable to finalize MD5 calculation" );
		raw.hashAlgorithm = static_cast< unsigned char >( Tongue::HashAlgorithm::MD5 );
		return raw;
	}

	static Hash MD5( const boost::filesystem::path & filename )
	{
		struct Tongue::Hash raw;
		MD5_CTX context;
		int result = MD5_Init( & context );
		if ( result != 1 )
			THROW( Error, "Unable to init MD5 calculation" );
		FileDescriptor descriptor( FileDescriptor::openForRead( filename.string().c_str() ) );
		unsigned char buffer[ 4096 ];
		ssize_t bytes;
		while ( true ) {
			bytes = read( descriptor.fd(), buffer, sizeof( buffer ) );
			if ( bytes < 0 )
				THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to read from " << filename );
			if ( bytes == 0 )
				break;
			result = MD5_Update( & context, buffer, bytes );
			if ( result != 1 )
				THROW( Error, "Unable to continue MD5 calculation" );
		}
		ASSERT( sizeof( raw.hash ) >= MD5_DIGEST_LENGTH );
		result = MD5_Final( raw.hash, & context );
		if ( result != 1 )
			THROW( Error, "Unable to finalize MD5 calculation" );
		raw.hashAlgorithm = static_cast< unsigned char >( Tongue::HashAlgorithm::MD5 );
		return raw;
	}

	static Hash SHA1( const void * buffer, unsigned length )
	{
		static Tongue::Hash raw;
		SHA_CTX context;
		int result = SHA1_Init( & context );
		if ( result != 1 )
			THROW( Error, "Unable to init SHA1 calculation" );
		result = SHA1_Update( & context, buffer, length );
		if ( result != 1 )
			THROW( Error, "Unable to continue SHA1 calculation" );
		ASSERT( sizeof( raw.hash ) >= SHA_DIGEST_LENGTH );
		result = SHA1_Final( raw.hash, & context );
		if ( result != 1 )
			THROW( Error, "Unable to finalize SHA1 calculation" );
		raw.hashAlgorithm = static_cast< unsigned char >( Tongue::HashAlgorithm::SHA1 );
		return raw;
	}

	static Hash SHA1( const boost::filesystem::path & filename )
	{
		struct Tongue::Hash raw;
		SHA_CTX context;
		int result = SHA1_Init( & context );
		if ( result != 1 )
			THROW( Error, "Unable to init SHA1 calculation" );
		FileDescriptor descriptor( FileDescriptor::openForRead( filename.string().c_str() ) );
		unsigned char buffer[ 4096 ];
		ssize_t bytes;
		while ( true ) {
			bytes = read( descriptor.fd(), buffer, sizeof( buffer ) );
			if ( bytes < 0 )
				THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to read from " << filename );
			if ( bytes == 0 )
				break;
			result = SHA1_Update( & context, buffer, bytes );
			if ( result != 1 )
				THROW( Error, "Unable to continue SHA1 calculation" );
		}
		ASSERT( sizeof( raw.hash ) >= SHA_DIGEST_LENGTH );
		result = SHA1_Final( raw.hash, & context );
		if ( result != 1 )
			THROW( Error, "Unable to finalize SHA1 calculation" );
		raw.hashAlgorithm = static_cast< unsigned char >( Tongue::HashAlgorithm::SHA1 );
		return raw;
	}

	static bool verify( const boost::filesystem::path & filename, const Hash & hash )
	{
		if ( hash.algorithm() == Tongue::HashAlgorithm::MD5 ) {
			Hash calculated = MD5( filename );
			return memcmp( calculated.bytes(), hash.bytes(), hash.bytesCount() ) == 0;
		} else {
			ASSERT( hash.algorithm() == Tongue::HashAlgorithm::SHA1 );
			Hash calculated = SHA1( filename );
			return memcmp( calculated.bytes(), hash.bytes(), hash.bytesCount() ) == 0;
		}
	}

	static bool verify( const void * buffer, unsigned length, const Hash & hash )
	{
		if ( hash.algorithm() == Tongue::HashAlgorithm::MD5 ) {
			Hash calculated = MD5( buffer, length );
			return memcmp( calculated.bytes(), hash.bytes(), hash.bytesCount() ) == 0;
		} else {
			ASSERT( hash.algorithm() == Tongue::HashAlgorithm::SHA1 );
			Hash calculated = SHA1( buffer, length );
			return memcmp( calculated.bytes(), hash.bytes(), hash.bytesCount() ) == 0;
		}
	}

private:
	CalculateHash( const CalculateHash & rhs ) = delete;
	CalculateHash & operator= ( const CalculateHash & rhs ) = delete;
};

} // namespace Osmosis

#endif // __OSMOSIS_CALCULATE_HASH_H__
