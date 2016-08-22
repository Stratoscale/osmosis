#ifndef __OSMOSIS_CALCULATE_HASH_H__
#define __OSMOSIS_CALCULATE_HASH_H__

#include <boost/filesystem.hpp>
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace CalculateHash
{

Hash MD5( const void * buffer, unsigned length );

Hash MD5( const boost::filesystem::path & filename );

Hash SHA1( const void * buffer, unsigned length );

Hash SHA1( const boost::filesystem::path & filename );

bool verify( const boost::filesystem::path & filename, const Hash & hash );

bool verify( const void * buffer, unsigned length, const Hash & hash );


} // namespace CalculateHash
} // namespace Osmosis

#endif // __OSMOSIS_CALCULATE_HASH_H__
