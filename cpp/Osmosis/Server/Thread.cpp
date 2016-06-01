#include <boost/asio/ip/tcp.hpp>
#include "Osmosis/Server/Thread.h"
#include "Osmosis/Server/GetOp.h"
#include "Osmosis/Server/PutOp.h"
#include "Osmosis/Server/IsExistsOp.h"
#include "Osmosis/Server/VerifyOp.h"
#include "Osmosis/Server/GetLabelOp.h"
#include "Osmosis/Server/SetLabelOp.h"
#include "Osmosis/Server/ListLabelsOp.h"
#include "Osmosis/Server/EraseLabelOp.h"
#include "Osmosis/Server/RenameLabelOp.h"
#include "Osmosis/Server/GetSupportedProtocolVersionsOp.h"
#include "Osmosis/Server/UpgradeProtocolOp.h"


namespace Osmosis {
namespace Server
{

Thread::Thread( boost::filesystem::path  rootPath,
	ObjectStore::Store &     store,
	ObjectStore::Drafts &    drafts,
	ObjectStore::Labels &    labels ) :
	_rootPath( rootPath ),
	_store( store ),
	_drafts( drafts ),
	_labels( labels ),
	_boostSocket( _ioService ),
	_socket( _boostSocket ),
	_protocolVersion( Tongue::MIN_SUPPORTED_PROTOCOL_VERSION ),
	_tcpNoDelay( false )
{}

boost::asio::ip::tcp::socket & Thread::socketForAccept()
{
	ASSERT( not _boostSocket.is_open() );
	return _boostSocket;
}

void Thread::run()
{
	BACKTRACE_BEGIN
	ASSERT( _boostSocket.is_open() );
	TRACE_INFO( "Connected to " << _boostSocket.remote_endpoint() << ", starting server thread" );
	setTCPNoDelay( true );
	setTCPKeepalive( true );
	std::thread thread( & Thread::thread, shared_from_this() );
	thread.detach();
	BACKTRACE_END
}

void Thread::thread()
{
	auto endpoint = _boostSocket.remote_endpoint();
	try {
		handshake();
		while ( work() );
		TRACE_INFO( "Server thread for " << endpoint << " done serving" );
	} CATCH_ALL_IGNORE( "Server thread for " << endpoint << " aborted" );
}

bool Thread::work()
{
	struct Tongue::Header header;
	try {
		header = _socket.receiveAll< struct Tongue::Header >();
	} catch ( boost::system::system_error & e ) {
		if ( e.code() == boost::asio::error::eof )
			return false;
		throw;
	}
	TRACE_DEBUG( "Incoming opcode: " << static_cast< unsigned >( header.opcode ) );
	switch ( static_cast< Tongue::Opcode >( header.opcode ) ) {
		case Tongue::Opcode::GET:
// TODO: Test for large transfers to decide if worth it
//				if ( not _tcpNoDelay )
//					setTCPNoDelay( false );
			GetOp( _socket, _store ).go();
			break;
		case Tongue::Opcode::PUT:
			PutOp( _socket, _store, _drafts ).go();
			break;
		case Tongue::Opcode::IS_EXISTS:
			IsExistsOp( _socket, _store ).go();
			break;
		case Tongue::Opcode::VERIFY:
			VerifyOp( _socket, _store ).go();
			break;
		case Tongue::Opcode::SET_LABEL:
			SetLabelOp( _socket, _labels ).go();
			break;
		case Tongue::Opcode::GET_LABEL:
			GetLabelOp( _socket, _labels ).go();
			break;
		case Tongue::Opcode::LIST_LABELS:
			ListLabelsOp( _socket, _labels ).go();
			break;
		case Tongue::Opcode::ERASE_LABEL:
			EraseLabelOp( _socket, _store, _labels ).go();
			break;
		case Tongue::Opcode::RENAME_LABEL:
			RenameLabelOp( _socket, _labels ).go();
			break;
		case Tongue::Opcode::ACK:
			THROW( Error, "Unexpected ack" );
			break;
		case Tongue::Opcode::GET_SUPPORTED_PROTOCOL_VERSIONS:
			GetSupportedProtocolVersionsOp( _socket ).go();
			break;
		case Tongue::Opcode::UPGRADE_PROTOCOL_VERSION:
			UpgradeProtocolOp( _socket, _protocolVersion ).go();
			break;
		default:
			THROW( Error, "Protocol Error: unknown opcode: " << header.opcode );
			break;
	}
	return true;
}

void Thread::handshake()
{
	BACKTRACE_BEGIN
	auto handshake = _socket.receiveAll< struct Tongue::Handshake >();
	const auto minSupportedVersion = static_cast< unsigned >( Tongue::MIN_SUPPORTED_PROTOCOL_VERSION );
	const auto maxSupportedVersion = static_cast< unsigned >( Tongue::MAX_SUPPORTED_PROTOCOL_VERSION );
	if ( handshake.protocolVersion < minSupportedVersion or handshake.protocolVersion > maxSupportedVersion  )
		THROW( Error, "Client with protocol version " << handshake.protocolVersion <<
				" attempted to connect to us, but the server only supports versions from " <<
				Tongue::MIN_SUPPORTED_PROTOCOL_VERSION << " to " << Tongue::MAX_SUPPORTED_PROTOCOL_VERSION );
	_protocolVersion = handshake.protocolVersion;
	if ( handshake.compression != static_cast< unsigned >( Tongue::Compression::UNCOMPRESSED ) )
		THROW( Error, "Compression not yet implemented" );
	Stream::AckOps( _socket ).sendAck();
	BACKTRACE_END
}

void Thread::setTCPNoDelay( bool value )
{
	boost::asio::ip::tcp::no_delay option( value );
	_boostSocket.set_option( option );
	_tcpNoDelay = value;
}

void Thread::setTCPKeepalive( bool value )
{
	boost::asio::socket_base::keep_alive option( value );
	_boostSocket.set_option( option );
}

} // namespace Server
} // namespace Osmosis
