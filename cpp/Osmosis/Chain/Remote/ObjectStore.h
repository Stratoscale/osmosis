#ifndef __OSMOSIS_CHAIN_REMOTE_OBJECT_STORE_H__
#define __OSMOSIS_CHAIN_REMOTE_OBJECT_STORE_H__

#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Remote/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

class ObjectStore : public ObjectStoreInterface
{
public:
	ObjectStore( const std::string & hostname, unsigned short port );

	std::unique_ptr< ObjectStoreConnectionInterface > connect() override;

private:
	const std::string     _hostname;
	const unsigned short  _port;

	ObjectStore( const ObjectStore & rhs ) = delete;
	ObjectStore & operator= ( const ObjectStore & rhs ) = delete;
};

} // namespace Remote
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_REMOTE_OBJECT_STORE_H__
