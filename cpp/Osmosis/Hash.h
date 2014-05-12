#ifndef __OSMOSIS_HASH_H__
#define __OSMOSIS_HASH_H__

#include <openssl/md5.h>
#include <openssl/sha.h>
#include "Osmosis/Tongue.h"
#include "Common/Error.h"
#include "Osmosis/Hex.h"

namespace Osmosis
{

class Hash
{
public:
	Hash( const struct Tongue::Hash & raw ) :
		_raw( raw )
	{
		ASSERT( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) or
				_raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 ) );
	}

	static Hash fromHex( const std::string & hex )
	{
		struct Tongue::Hash raw;
		Hex::toBuffer( hex, raw.hash, sizeof( raw.hash ) );
		if ( hex.size() == MD5_DIGEST_LENGTH * 2 )
			raw.hashAlgorithm = static_cast< unsigned >( Tongue::HashAlgorithm::MD5 );
		else if ( hex.size() == SHA_DIGEST_LENGTH * 2 )
			raw.hashAlgorithm = static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 );
		else
			THROW( Error, "Hash hex strings can be only 32 characters for MD5, or 40 for SHA1, but "
					"string " << hex << " is " << hex.size() );
		return raw;
	}

	boost::filesystem::path relativeFilename() const
	{
		char name[ 64 ];
		snprintf( name, sizeof( name ), "%02x", _raw.hash[ 0 ] );
		boost::filesystem::path result( name );
		snprintf( name, sizeof( name ), "%02x", _raw.hash[ 1 ] );
		result /= name;
		Hex::fromBuffer( _raw.hash + 2, bytesCount() - 2, name, sizeof( name ) );
		result /= name;
		return result;
	}

	Tongue::HashAlgorithm algorithm() const
	{
		return static_cast< Tongue::HashAlgorithm >( _raw.hashAlgorithm );
	}

	const unsigned char * bytes() const
	{
		return _raw.hash;
	}

	unsigned bytesCount() const
	{
		ASSERT( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) or
				_raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::SHA1 ) );
		return ( _raw.hashAlgorithm == static_cast< unsigned >( Tongue::HashAlgorithm::MD5 ) ) ? 16 : 20;
	}

	friend std::ostream & operator<<( std::ostream & os, const Hash & hash )
	{
		char buffer[ 64 ];
		Hex::fromBuffer( hash._raw.hash, hash.bytesCount(), buffer, sizeof( buffer ) );
		os << buffer;
		return os;
	}

	const Tongue::Hash & raw() const
	{
		return _raw;
	}

	bool operator == ( const Hash & other ) const
	{
		return _raw.hashAlgorithm == other._raw.hashAlgorithm and
			memcmp( _raw.hash, other._raw.hash, bytesCount() ) == 0;
	}

	bool operator != ( const Hash & other ) const
	{
		return not operator == ( other );
	}

	bool operator < ( const Hash & other ) const
	{
		return value() < other.value();
	}

private:
	struct Tongue::Hash _raw;

	unsigned long value() const
	{
		return * reinterpret_cast< const unsigned long * >( & _raw );
	}
};

} // namespace Osmosis

#endif // __OSMOSIS_HASH_H__
