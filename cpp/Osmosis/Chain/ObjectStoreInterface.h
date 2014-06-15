#ifndef __OSMOSIS_CHAIN_OBJECT_STORE_INTERFACE_H__
#define __OSMOSIS_CHAIN_OBJECT_STORE_INTERFACE_H__

#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"

namespace Osmosis {
namespace Chain
{

class ObjectStoreInterface
{
public:
	ObjectStoreInterface() = default;
	virtual ~ObjectStoreInterface() = default;
	virtual std::unique_ptr< ObjectStoreConnectionInterface > connect() = 0;

private:
	ObjectStoreInterface( const ObjectStoreInterface & rhs ) = delete;
	ObjectStoreInterface & operator= ( const ObjectStoreInterface & rhs ) = delete;
};

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_OBJECT_STORE_INTERFACE_H__
