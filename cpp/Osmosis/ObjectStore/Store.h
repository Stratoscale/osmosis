#ifndef __OSMOSIS_OBJECT_STORE_STORE_H__
#define __OSMOSIS_OBJECT_STORE_STORE_H__

#include "Osmosis/Hash.h"
#include "Osmosis/ObjectStore/ObjectsIterator.h"

namespace Osmosis {
namespace ObjectStore
{

class Store
{
public:
	Store( const boost::filesystem::path & rootPath );

	bool exists( const Hash & hash ) const;

	void verifyOrDestroy( const Hash & hash ) const;

	boost::filesystem::path filenameForExisting( const Hash & hash ) const;

	void putExistingFile( const Hash & hash, const boost::filesystem::path & filename );

	void erase( const Hash & hash );

	ObjectsIterator list() const;

	ObjectsIterator list( boost::filesystem::path rootDir ) const;

private:
	boost::filesystem::path _rootPath;

	boost::filesystem::path absoluteFilename( const Hash & hash ) const;

	void recoverInCaseOfDirectoryCorruption( boost::filesystem::path dirPath );

	Store( const Store & rhs ) = delete;
	Store & operator= ( const Store & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_STORE_H__
