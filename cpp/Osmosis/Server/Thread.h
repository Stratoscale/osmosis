#ifndef __OSMOSIS_SERVER_THREAD_H__
#define __OSMOSIS_SERVER_THREAD_H__

#include <boost/filesystem.hpp>
#include <thread>
#include "Osmosis/Server/GetOp.h"
#include "Osmosis/Server/PutOp.h"
#include "Osmosis/Server/IsExistsOp.h"
#include "Osmosis/Server/GetLabelOp.h"
#include "Osmosis/Server/SetLabelOp.h"

namespace Osmosis {
namespace Server
{

class Thread : public std::enable_shared_from_this< Thread >
{
public:
	Thread( boost::filesystem::path  rootPath,
		ObjectStore::Store &     store,
		ObjectStore::Drafts &    drafts,
		ObjectStore::Labels &    labels ) :
		_rootPath( rootPath ),
		_store( store ),
		_drafts( drafts ),
		_labels( labels ),
		_boostSocket( _ioService ),
		_socket( _boostSocket )
	{}

	boost::asio::ip::tcp::socket & socketForAccept()
	{
		ASSERT( not _boostSocket.is_open() );
		return _boostSocket;
	}

	void run()
	{
		ASSERT( _boostSocket.is_open() );
		TRACE_INFO( "Connected to " << _boostSocket.remote_endpoint() << ", starting server thread" );
		boost::asio::ip::tcp::no_delay option( true );
		_boostSocket.set_option(option);
		std::thread thread( & Thread::thread, shared_from_this() );
		thread.detach();
	}

private:
	boost::filesystem::path       _rootPath;
	ObjectStore::Store &          _store;
	ObjectStore::Drafts &         _drafts;
	ObjectStore::Labels &         _labels;
	boost::asio::io_service       _ioService;
	boost::asio::ip::tcp::socket  _boostSocket;
	TCPSocket                     _socket; 

	void thread()
	{
		try {
			while ( work() );
			TRACE_INFO( "Server thread for " << _boostSocket.remote_endpoint() << " done serving" );
		} catch ( boost::exception & e ) {
			TRACE_BOOST_EXCEPTION( e, "Server thread for " << _boostSocket.remote_endpoint() <<
					" terminated on a boost exception" );
		} catch ( Error & e ) {
			TRACE_ERROR( "Server thread for " << _boostSocket.remote_endpoint() <<
					" terminated on 'Error' exception: '" << e.what() <<
					"' at " << e.filename << ':' << e.line );
		} catch ( std::exception & e ) {
			TRACE_ERROR( "Server thread for " << _boostSocket.remote_endpoint() << 
					" terminated on std::exception: '" << e.what() << "'" );
		} catch ( ... ) {
			TRACE_ERROR( "Server thread for " << _boostSocket.remote_endpoint() <<
					" terminated on unknown exception" );
		}
	}

	bool work()
	{
		struct Tongue::Header header;
		try {
			header = _socket.recieveAll< struct Tongue::Header >();
		} catch ( boost::system::system_error & e ) {
			if ( e.code() == boost::asio::error::eof )
				return false;
			throw;
		}
		switch ( static_cast< Tongue::Opcode >( header.opcode ) ) {
			case Tongue::Opcode::GET:
				GetOp( _socket, _store ).go();
				break;
			case Tongue::Opcode::PUT:
				PutOp( _socket, _store, _drafts ).go();
				break;
			case Tongue::Opcode::IS_EXISTS:
				IsExistsOp( _socket, _store ).go();
				break;
			case Tongue::Opcode::SET_LABEL:
				SetLabelOp( _socket, _labels ).go();
				break;
			case Tongue::Opcode::GET_LABEL:
				GetLabelOp( _socket, _labels ).go();
				break;
			case Tongue::Opcode::ACK:
				THROW( Error, "Unexpected ack" );
				break;
			default:
				THROW( Error, "Protocol Error: unknown opcode: " << header.opcode );
				break;
		}
		return true;
	}

	Thread( const Thread & rhs ) = delete;
	Thread & operator= ( const Thread & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_THREAD_H__
