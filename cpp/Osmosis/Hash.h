#ifndef __OSMOSIS_HASH_H__
#define __OSMOSIS_HASH_H__

#include <ostream>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <boost/filesystem/path.hpp>
#include "Osmosis/Tongue.h"
#include "Common/Error.h"
#include "Osmosis/Hex.h"

namespace Osmosis
{

class Hash
{
public:
	Hash( const struct Tongue::Hash & raw );

	Hash( const char *hex, unsigned length );

	Hash( const std::string & hex );

	Hash();

	boost::filesystem::path relativeFilename() const;

	Tongue::HashAlgorithm algorithm() const;

	const unsigned char * bytes() const;

	unsigned bytesCount() const;

	const Tongue::Hash & raw() const;

	friend std::ostream & operator<<( std::ostream & os, const Hash & hash );

	friend bool operator == ( const Hash & first, const Hash & other );

	friend bool operator != ( const Hash & first, const Hash & other );

	friend bool operator < ( const Hash & first, const Hash & other );

private:
	struct Tongue::Hash _raw;

	unsigned long value() const;
};

} // namespace Osmosis

namespace std
{

template<> struct hash< Osmosis::Hash >
{
	std::size_t operator()( const Osmosis::Hash & hash ) const
	{
		return * reinterpret_cast< const std::size_t * >( hash.bytes() );
	}
};

} // namespace std

#endif // __OSMOSIS_HASH_H__
