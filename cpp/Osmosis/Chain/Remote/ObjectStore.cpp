#include "Osmosis/Chain/Remote/ObjectStore.h"
#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Remote/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

ObjectStore::ObjectStore( const std::string & hostname, unsigned short port, unsigned int tcpTimeout ):
	_hostname( hostname ),
	_port( port ),
	_tcpTimeout( tcpTimeout )
{}

std::unique_ptr< ObjectStoreConnectionInterface > ObjectStore::connect()
{
	return std::unique_ptr< ObjectStoreConnectionInterface >( new Connection( _hostname, _port,
		_tcpTimeout) );
}

} // namespace Remote
} // namespace Chain
} // namespace Osmosis
