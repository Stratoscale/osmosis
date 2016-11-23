#ifndef __OSMOSIS_CLIENT_WHO_HAS_LABEL_H__
#define __OSMOSIS_CLIENT_WHO_HAS_LABEL_H__

#include <string>
#include <list>
#include "Osmosis/UDPSocket.h"

namespace Osmosis {
namespace Client
{

class WhoHasLabel
{
public:
	WhoHasLabel( const std::string & label, unsigned short timeoutInMilliseconds,
	             unsigned short remoteListeningPort, bool broadcastToLocalhost );

	std::list< std::string > go();

private:
	const std::string                      _label;
	const unsigned short                   _timeoutInMilliseconds;
	const unsigned short                   _localListeningPort;
	const unsigned short                   _remoteListeningPort;
	UDPSocket                              _socket;
	unsigned char                          _buffer[1024];

	void broadcastRequest();
	std::list< std::string > receiveObjectStores();

	WhoHasLabel( const WhoHasLabel & rhs ) = delete;
	WhoHasLabel & operator= ( const WhoHasLabel & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_WHO_HAS_LABEL_H__
