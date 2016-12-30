#include "Osmosis/Chain/Factory.h"

namespace Osmosis {
namespace Chain
{

std::unique_ptr< ObjectStoreInterface > factory( const std::string & location, unsigned int tcpTimeout )
{
	BACKTRACE_BEGIN
	if ( location.size() == 0 )
		THROW( Error, "Location '" << location << "' is in invalid format: zero length" );
	if ( location[ 0 ] == '/' ) {
		boost::filesystem::path path( location );
		return std::unique_ptr< ObjectStoreInterface >( new Local::ObjectStore( path ) );
	} else if ( boost::starts_with( location, "http://" ) ) {
		return std::unique_ptr< ObjectStoreInterface >( new Http::ObjectStore( location ) );
	} else {
		std::vector< std::string > split;
		boost::split( split, location, boost::is_any_of( ":" ) );
		if ( split.size() != 2 )
			THROW( Error, "Location '" << location << "' is in invalid format: expected '<hostname>:<port>' format" );
		int port = std::stoi( split[ 1 ] );
		if ( port < 1 or port >= ( 1 << 16 ) )
			THROW( Error, "Port '" << location << "' must be in the range 1-65535" );
		return std::unique_ptr< ObjectStoreInterface >(
			new Remote::ObjectStore( split[ 0 ], (unsigned short) port, tcpTimeout ) );
	}
	BACKTRACE_END
}

} // namespace Chain
} // namespace Osmosis
