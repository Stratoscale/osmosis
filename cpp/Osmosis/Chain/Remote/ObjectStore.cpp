#include "Osmosis/Chain/Remote/ObjectStore.h"
#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Remote/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

ObjectStore::ObjectStore( const std::string & hostname, unsigned short port ):
	_hostname( hostname ),
	_port( port )
{}

std::unique_ptr< ObjectStoreConnectionInterface > ObjectStore::connect()
{
	return std::unique_ptr< ObjectStoreConnectionInterface >( new Connection( _hostname, _port ) );
}

} // namespace Remote
} // namespace Chain
} // namespace Osmosis
