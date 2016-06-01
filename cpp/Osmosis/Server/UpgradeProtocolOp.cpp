#include <Osmosis/Server/UpgradeProtocolOp.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>
#include <Osmosis/Debug.h>
#include <Osmosis/Stream/AckOps.h>
#include "Common/Error.h"

namespace Osmosis {
namespace Server
{

UpgradeProtocolOp::UpgradeProtocolOp( TCPSocket & socket, uint32_t &protocolVersion ):
		_socket( socket ),
		_protocolVersion( protocolVersion )
{}

void UpgradeProtocolOp::go()
{
	auto requestedProtocol = _socket.receiveAll< uint32_t >();
	const auto minSupportedVersion = static_cast< unsigned >( Tongue::MIN_SUPPORTED_PROTOCOL_VERSION );
	const auto maxSupportedVersion = static_cast< unsigned >( Tongue::MAX_SUPPORTED_PROTOCOL_VERSION );
	if ( requestedProtocol < minSupportedVersion or requestedProtocol > maxSupportedVersion  )
		THROW( Error, "Cannot upgrade protocol to version " << requestedProtocol << "."
				" The server only supports versions from " <<
				Tongue::MIN_SUPPORTED_PROTOCOL_VERSION << "to " << Tongue::MAX_SUPPORTED_PROTOCOL_VERSION );
	_protocolVersion = requestedProtocol;
	Stream::AckOps( _socket ).sendAck();
}

} // namespace Server
} // namespace Osmosis
