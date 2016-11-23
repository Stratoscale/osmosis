#include <array>
#include <boost/asio/placeholders.hpp>
#include "Osmosis/Tongue.h"
#include "Common/Debug.h"
#include "Osmosis/Server/BroadcastServer.h"

namespace Osmosis {
namespace Server
{

BroadcastServer::BroadcastServer( boost::asio::io_service & ioService,
								  boost::filesystem::path & rootPath,
								  unsigned short port ):
	_socket( ioService, boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), port ) ),
	_labels( rootPath, _store ),
	_store( rootPath ) {
	using Tongue::Opcode;
	memset( _commandHandlers, 0, sizeof( _commandHandlers ) );
	_commandHandlers[ static_cast< int > ( Opcode::WHO_HAS_LABEL ) ] = &BroadcastServer::doesLabelExist;
	asyncReceive();
	TRACE_INFO( "Broadcast-server listening on port " << port << "..." );
}

void BroadcastServer::asyncReceive() {
	using boost::asio::ip::udp;
	boost::system::error_code error;
	SocketHandler receiveHandlerPtr = &BroadcastServer::receiveHandler;
	_socket.async_receive_from( boost::asio::buffer( _recvBuffer ),
								_remoteEndpoint,
								boost::bind( receiveHandlerPtr,
											 this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred ) );
}

void BroadcastServer::receiveHandler( const boost::system::error_code& error,
	std::size_t messageSize ) {
	using Tongue::Opcode;
	if ( error && error != boost::asio::error::message_size ) {
		TRACE_INFO( error );
		throw boost::system::system_error( error );
	}
	asyncReceive();
	if (0 == messageSize) {
		TRACE_WARNING( "Broadcast-server received an empty message from " << _remoteEndpoint );
		return;
	}
	struct Tongue::Header header { _recvBuffer[0] };
	Opcode opcode = static_cast< Opcode > ( header.opcode );
	if ( opcode > Opcode::MAX_OPCODE ) {
		TRACE_WARNING( "Broadcast-server received an invalid opcode: " <<
						static_cast<int> ( header.opcode ) << " from " << _remoteEndpoint );
		return;
	}
	int opcodeNr = static_cast< int > ( opcode );
	CommandHandler handler = _commandHandlers[ opcodeNr ];
	if ( handler == nullptr ) {
		TRACE_WARNING( "Requests of type " << opcodeNr << " are not implemented in broadcast-server" );
		return;
	}
	TRACE_INFO( "Serving a broadcast request of type " << opcodeNr << " from " << _remoteEndpoint );
	unsigned char * payload = & _recvBuffer[1];
	const auto payloadSize = messageSize - 1;
	( this->*handler )( _remoteEndpoint, payload, payloadSize );
}

void BroadcastServer::sendHandler( const boost::system::error_code& error, std::size_t,
				  boost::asio::ip::udp::endpoint remoteEndpoint ) {
	if ( error && error != boost::asio::error::message_size ) {
		TRACE_WARNING( "An error has occurred while responding to a broadcast request:" );
		TRACE_WARNING( error );
		return;
	}
	TRACE_INFO( "Successfuly served broadcast request by endpoint: " << remoteEndpoint );
}

#include <unistd.h>
void BroadcastServer::doesLabelExist( boost::asio::ip::udp::endpoint remoteEndpoint,
		unsigned char * message,
		std::size_t messageSize ) {
	if ( messageSize <= sizeof( Tongue::Label ) ) {
		TRACE_WARNING( "does-list-label received an message with an invalid size: " << messageSize <<
		               " (too small)" );
		return;
	}
	Tongue::Label * labelHeader = reinterpret_cast< Tongue::Label * > ( message );
	if ( labelHeader->length != messageSize - sizeof( Tongue::Label ) ) {
		TRACE_WARNING( "does-list-label received an message with an invalid size: " << messageSize <<
		               " (does not match the message size)" );
		return;
	}
	std::string label( reinterpret_cast< char * > ( labelHeader->label ), labelHeader->length );
	const bool result = _labels.exists( label );
	if ( result ) {
		static Tongue::IsExistsResponse exists {
			static_cast< unsigned char > ( Tongue::IsExists::YES )
		};
		_socket.async_send_to( boost::asio::buffer( & exists, sizeof( exists ) ),
							   _remoteEndpoint,
							   boost::bind( &BroadcastServer::sendHandler,
											this,
											boost::asio::placeholders::error,
											boost::asio::placeholders::bytes_transferred,
											remoteEndpoint ) );
	}
}

} // namespace Server
} // namespace Osmosis
