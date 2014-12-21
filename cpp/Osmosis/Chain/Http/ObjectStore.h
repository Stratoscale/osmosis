#ifndef __OSMOSIS_CHAIN_HTTP_OBJECT_STORE_H__
#define __OSMOSIS_CHAIN_HTTP_OBJECT_STORE_H__

#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Http/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Http
{

class ObjectStore : public ObjectStoreInterface
{
public:
	ObjectStore( const std::string & url ):
		_url( url )
	{
		ASSERT( boost::starts_with( url, "http://" ) );
	}

	std::unique_ptr< ObjectStoreConnectionInterface > connect() override
	{
		return std::unique_ptr< ObjectStoreConnectionInterface >( new Connection( _url ) );
	}

private:
	const std::string _url;

	ObjectStore( const ObjectStore & rhs ) = delete;
	ObjectStore & operator= ( const ObjectStore & rhs ) = delete;
};

} // namespace Http
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_HTTP_OBJECT_STORE_H__
