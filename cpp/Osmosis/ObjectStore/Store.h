#ifndef __OSMOSIS_OBJECT_STORE_STORE_H__
#define __OSMOSIS_OBJECT_STORE_STORE_H__

#include "Osmosis/Hash.h"
#include "Osmosis/CalculateHash.h"

namespace Osmosis {
namespace ObjectStore
{

class Store
{
public:
	Store( const boost::filesystem::path & rootPath ) :
		_rootPath( rootPath )
	{}

	bool exists( const Hash & hash ) const
	{
		return boost::filesystem::exists( absoluteFilename( hash ) );
	}

	boost::filesystem::path filenameForExisting( const Hash & hash ) const
	{
		ASSERT( exists( hash ) );
		return absoluteFilename( hash );
	}

	void putExistingFile( const Hash & hash, const boost::filesystem::path & filename )
	{
		if ( not CalculateHash::verify( filename, hash ) )
			THROW( Error, "Will not put file that does not match it's hash " << hash );
		boost::filesystem::path absolute = absoluteFilename( hash );
		boost::filesystem::create_directories( absolute.parent_path() );
		boost::filesystem::rename( filename, absolute );
	}

private:
	boost::filesystem::path _rootPath;

	boost::filesystem::path absoluteFilename( const Hash & hash ) const
	{
		return _rootPath / hash.relativeFilename();
	}

	Store( const Store & rhs ) = delete;
	Store & operator= ( const Store & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_STORE_H__
