#include <algorithm>
#include <boost/asio/error.hpp>
#include <Osmosis/Chain/Remote/ProtocolVersionNegotiator.h>
#include <Osmosis/TCPConnection.h>
#include <Osmosis/Tongue.h>
#include <Osmosis/Stream/AckOps.h>

namespace Osmosis {
namespace Chain {
namespace Remote
{

ProtocolVersionNegotiator::ProtocolVersionNegotiator( TCPConnection & connection,
		uint32_t currentProtocol ) :
	_connection( connection ),
	_currentProtocol( currentProtocol )
{}

Tongue::SupportedProtocolVersions ProtocolVersionNegotiator::getSupportedVersionsInServer() {
	const struct Tongue::Header header = { static_cast< unsigned char >(
			Tongue::Opcode::GET_SUPPORTED_PROTOCOL_VERSIONS ) };
	_connection.socket().sendAll( header );
	struct Tongue::SupportedProtocolVersions response = { Tongue::FIRST_PROTOCOL_VERSION,
														  Tongue::FIRST_PROTOCOL_VERSION };
	try {
		response = _connection.socket().receiveAll< struct Tongue::SupportedProtocolVersions >();
	} catch( boost::system::system_error &ex ) {
		if ( ex.code().value() == boost::asio::error::misc_errors::eof ) {
			// Nothing to do. The server does not support this op
			_connection.connect();
		} else {
			TRACE_ERROR("An unknown error (code: " << ex.code() << ") has occurred while trying to get the "
					    << "list of supported protocols from the server.");
			throw ex;
		}
	}
	return response;
}

uint32_t ProtocolVersionNegotiator::getMaxCommonSupportedProtocolVersion() {
	Tongue::SupportedProtocolVersions serverVersions;
	serverVersions = getSupportedVersionsInServer();
	const uint32_t maxCommonSupportedVersion = std::min( serverVersions.max,
			static_cast<uint32_t>(Tongue::MAX_SUPPORTED_PROTOCOL_VERSION) );
	if ( maxCommonSupportedVersion < Tongue::MIN_SUPPORTED_PROTOCOL_VERSION  or
			maxCommonSupportedVersion < serverVersions.min ) {
		return Tongue::FIRST_PROTOCOL_VERSION;
	}
	return maxCommonSupportedVersion;
}

uint32_t ProtocolVersionNegotiator::negotiate() {
	const uint32_t maxCommonSupportedVersion = getMaxCommonSupportedProtocolVersion();
	if ( maxCommonSupportedVersion > _currentProtocol ) {
		ASSERT( maxCommonSupportedVersion > Tongue::FIRST_PROTOCOL_VERSION );
		const struct Tongue::Header header =
			{ static_cast< unsigned char >( Tongue::Opcode::UPGRADE_PROTOCOL_VERSION ) };
		_connection.socket().sendAllConcated( header, maxCommonSupportedVersion );
		Stream::AckOps( _connection.socket() ).wait( "Protocol upgrade confirmation" );
		return maxCommonSupportedVersion;
	}
	return _currentProtocol;
}

} // namespace Remote
} // namespace Chain
} // namespace Osmosis
