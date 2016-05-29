#ifndef __OSMOSIS_CHAIN_OBJECT_STORE_CONNECTION_INTERFACE_H__
#define __OSMOSIS_CHAIN_OBJECT_STORE_CONNECTION_INTERFACE_H__

#include <list>
#include <string>
#include <boost/filesystem/path.hpp>
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace Chain
{

class ObjectStoreConnectionInterface
{
public:
	ObjectStoreConnectionInterface() = default;
	virtual ~ObjectStoreConnectionInterface() = default;

	virtual void putString( const std::string & blob, const Hash & hash ) = 0;
	virtual std::string getString( const Hash & hash ) = 0;
	virtual void putFile( const boost::filesystem::path & path, const Hash & hash ) = 0;
	virtual void getFile( const boost::filesystem::path & path, const Hash & hash ) = 0;
	virtual bool exists( const Hash & hash ) = 0;
	virtual void verify( const Hash & hash ) = 0;
	virtual void eraseLabel( const std::string & label ) = 0;
	virtual void setLabel( const Hash & hash, const std::string & label ) = 0;
	virtual Hash getLabel( const std::string & label ) = 0;
	virtual void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo ) = 0;
	virtual std::list< std::string > listLabels( const std::string & regex ) = 0;

private:
	ObjectStoreConnectionInterface( const ObjectStoreConnectionInterface & rhs ) = delete;
	ObjectStoreConnectionInterface & operator= ( const ObjectStoreConnectionInterface & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_OBJECT_STORE_CONNECTION_INTERFACE_H__
