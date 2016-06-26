#include <Osmosis/Server/GetSupportedProtocolVersionsOp.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>
#include "Common/Error.h"

namespace Osmosis {
namespace Server
{

GetSupportedProtocolVersionsOp::GetSupportedProtocolVersionsOp( TCPSocket & socket ):
	_socket( socket )
{}

void GetSupportedProtocolVersionsOp::go()
{
	BACKTRACE_BEGIN
	Tongue::SupportedProtocolVersions message = { Tongue::MIN_SUPPORTED_PROTOCOL_VERSION,
			Tongue::MAX_SUPPORTED_PROTOCOL_VERSION };
	_socket.sendAll( message );
	BACKTRACE_END
}

} // namespace Server
} // namespace Osmosis
