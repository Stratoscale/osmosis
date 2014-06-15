#ifndef __OSMOSIS_CHAIN_FACTORY_H__
#define __OSMOSIS_CHAIN_FACTORY_H__

#include "Osmosis/Chain/Remote/ObjectStore.h"

namespace Osmosis {
namespace Chain
{

std::unique_ptr< ObjectStoreInterface > factory( const std::string & location )
{
	if ( location.size() == 0 )
		THROW( Error, "Location '" << location << "' is in invalid format: zero length" );
	if ( location[ 0 ] == '/' ) {
ASSERT_VERBOSE( false, "Not Implemetned yet" );
THROW( Error, "Not implemented" );
	} else {
		std::vector< std::string > split;
		boost::split( split, location, boost::is_any_of( ":" ) );
		if ( split.size() != 2 )
			THROW( Error, "Location '" << location << "' is in invalid format: expected '<hostname>:<port>' format" );
		int port = std::stoi( split[ 1 ] );
		if ( port < 1 or port >= ( 1 << 16 ) )
			THROW( Error, "Port '" << location << "' must be in the range 1-65535" );
		return std::unique_ptr< ObjectStoreInterface >( new Remote::ObjectStore( split[ 0 ], (unsigned short) port ) );
	}
}

} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_FACTORY_H__
