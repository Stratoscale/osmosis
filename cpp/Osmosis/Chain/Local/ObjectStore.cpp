#include "Osmosis/Chain/Local/ObjectStore.h"
#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Local/Connection.h"

namespace Osmosis {
namespace Chain {
namespace Local
{

ObjectStore::ObjectStore( const boost::filesystem::path & path ) :
	_store( path ),
	_drafts( path ),
	_labels( path, _store )
{}

std::unique_ptr< ObjectStoreConnectionInterface > ObjectStore::connect()
{
	return std::unique_ptr< ObjectStoreConnectionInterface >( new Connection( _store, _drafts, _labels ) );
}

} // namespace Local
} // namespace Chain
} // namespace Osmosis
