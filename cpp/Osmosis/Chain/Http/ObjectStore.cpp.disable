#include "Osmosis/Chain/Http/ObjectStore.h"
#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Http/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Http
{

ObjectStore::ObjectStore( const std::string & url ):
	_url( url )
{
	ASSERT( boost::starts_with( url, "http://" ) );
}

std::unique_ptr< ObjectStoreConnectionInterface > ObjectStore::connect()
{
	return std::unique_ptr< ObjectStoreConnectionInterface >( new Connection( _url ) );
}

} // namespace Http
} // namespace Chain
} // namespace Osmosis
