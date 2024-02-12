#include "Osmosis/CalculateHash.h"
#include "Osmosis/FileDescriptor.h"
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace CalculateHash
{

Hash MD5( const void * buffer, unsigned long length )
{
	MD5_CTX context;
	int result = MD5_Init( & context );
	if ( result != 1 )
		THROW( Error, "Unable to init MD5 calculation" );
	result = MD5_Update( & context, buffer, length );
	if ( result != 1 ) THROW( Error, "Unable to continue MD5 calculation" );
	Tongue::Hash raw;
	ASSERT( sizeof( raw.hash ) >= MD5_DIGEST_LENGTH );
	result = MD5_Final( raw.hash, & context );
	if ( result != 1 )
		THROW( Error, "Unable to finalize MD5 calculation" );
	raw.hashAlgorithm = static_cast< unsigned char >( Tongue::HashAlgorithm::MD5 );
	return raw;
}

Hash MD5( const boost::filesystem::path & filename )
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

Hash SHA1( const void * buffer, unsigned long length )
{
	Tongue::Hash raw;
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

Hash SHA1( const boost::filesystem::path & filename )
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

bool verify( const boost::filesystem::path & filename, const Hash & hash )
{
	if ( hash.algorithm() == Tongue::HashAlgorithm::MD5 ) {
		Hash calculated = MD5( filename );
		return calculated == hash;
	} else {
		ASSERT( hash.algorithm() == Tongue::HashAlgorithm::SHA1 );
		Hash calculated = SHA1( filename );
		return calculated == hash;
	}
}

bool verify( const void * buffer, unsigned long length, const Hash & hash )
{
	if ( hash.algorithm() == Tongue::HashAlgorithm::MD5 ) {
		Hash calculated = MD5( buffer, length );
		return calculated == hash;
	} else {
		ASSERT( hash.algorithm() == Tongue::HashAlgorithm::SHA1 );
		Hash calculated = SHA1( buffer, length );
		return calculated == hash;
	}
}

calcHash _verify( const Tongue::HashAlgorithm & algorithm, )

} // namespace CalculateHash
} // namespace Osmosis
