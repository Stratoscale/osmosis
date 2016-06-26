#ifndef __OSMOSIS_PROTOCOL_NEGOTIATOR_H__
#define __OSMOSIS_PROTOCOL_NEGOTIATOR_H__

#include <cstdint>
#include <boost/filesystem/path.hpp>
#include "Osmosis/TCPConnection.h"
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

class ProtocolVersionNegotiator
{
public:
	ProtocolVersionNegotiator( TCPConnection & connection, uint32_t currentProtocol );

	uint32_t negotiate();

private:
	TCPConnection & _connection;
	uint32_t _currentProtocol;

	Tongue::SupportedProtocolVersions getSupportedVersionsInServer();

	uint32_t getMaxCommonSupportedProtocolVersion();

	ProtocolVersionNegotiator( const ProtocolVersionNegotiator & rhs ) = delete;
	ProtocolVersionNegotiator & operator= ( const ProtocolVersionNegotiator & rhs ) = delete;
};

} // namespace Remote
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_PROTOCOL_NEGOTIATOR_H__
