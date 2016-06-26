#ifndef OSMOSIS_SERVER_UPGRADE_PROTOCOL_OP_H_
#define OSMOSIS_SERVER_UPGRADE_PROTOCOL_OP_H_

#include <Osmosis/TCPSocket.h>
#include <Osmosis/Tongue.h>

namespace Osmosis {
namespace Server
{

class UpgradeProtocolOp
{
public:
	UpgradeProtocolOp( TCPSocket & socket, uint32_t &protocolVersion );

	void go();

private:
	TCPSocket &            _socket;
	uint32_t &             _protocolVersion;

	UpgradeProtocolOp( const UpgradeProtocolOp & rhs ) = delete;
	UpgradeProtocolOp & operator= ( const UpgradeProtocolOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif
