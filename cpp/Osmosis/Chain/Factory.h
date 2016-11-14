#ifndef __OSMOSIS_CHAIN_FACTORY_H__
#define __OSMOSIS_CHAIN_FACTORY_H__

#include "Osmosis/Chain/Remote/ObjectStore.h"
#include "Osmosis/Chain/Local/ObjectStore.h"
#include "Osmosis/Chain/Http/ObjectStore.h"

namespace Osmosis {
namespace Chain
{

std::unique_ptr< ObjectStoreInterface > factory( const std::string & location );

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_FACTORY_H__
