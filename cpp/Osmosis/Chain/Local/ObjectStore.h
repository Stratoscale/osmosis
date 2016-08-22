#ifndef __OSMOSIS_CHAIN_LOCAL_OBJECT_STORE_H__
#define __OSMOSIS_CHAIN_LOCAL_OBJECT_STORE_H__

#include <boost/filesystem.hpp>
#include "Osmosis/Chain/ObjectStoreInterface.h"
#include "Osmosis/Chain/Local/Connection.h"
#include "Osmosis/ObjectStore/Store.h"
#include "Osmosis/ObjectStore/Drafts.h"
#include "Osmosis/ObjectStore/Labels.h"

namespace Osmosis {
namespace Chain {
namespace Local
{

class ObjectStore : public ObjectStoreInterface
{
public:
	ObjectStore( const boost::filesystem::path & path );

	std::unique_ptr< ObjectStoreConnectionInterface > connect() override;

private:
	Osmosis::ObjectStore::Store   _store;
	Osmosis::ObjectStore::Drafts  _drafts;
	Osmosis::ObjectStore::Labels  _labels;

	ObjectStore( const ObjectStore & rhs ) = delete;
	ObjectStore & operator= ( const ObjectStore & rhs ) = delete;
};

} // namespace Local
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_LOCAL_OBJECT_STORE_H__
