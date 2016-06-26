#ifndef OSMOSIS_SERVER_GET_SUPPORTED_PROTOCOL_VERSIONS_OP_H_
#define OSMOSIS_SERVER_GET_SUPPORTED_PROTOCOL_VERSIONS_OP_H_

#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>

namespace Osmosis {
namespace Server
{

class GetSupportedProtocolVersionsOp
{
public:
	GetSupportedProtocolVersionsOp( TCPSocket & socket );

	void go();

private:
	TCPSocket &            _socket;

	GetSupportedProtocolVersionsOp( const GetSupportedProtocolVersionsOp & rhs ) = delete;
	GetSupportedProtocolVersionsOp & operator= ( const GetSupportedProtocolVersionsOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif
