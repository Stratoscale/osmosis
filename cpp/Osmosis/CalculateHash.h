#ifndef __OSMOSIS_CALCULATE_HASH_H__
#define __OSMOSIS_CALCULATE_HASH_H__

#include <boost/filesystem.hpp>
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace CalculateHash
{

Hash MD5( const void * buffer, unsigned long length );

Hash MD5( const boost::filesystem::path & filename );

Hash SHA1( const void * buffer, unsigned long length );

Hash SHA1( const boost::filesystem::path & filename );

bool verify( const boost::filesystem::path & filename, const Hash & hash );

bool verify( const void * buffer, unsigned long length, const Hash & hash );

typedef Hash (*hashHandler)( const void * buffer, unsigned long length);



} // namespace CalculateHash
} // namespace Osmosis

#endif // __OSMOSIS_CALCULATE_HASH_H__
