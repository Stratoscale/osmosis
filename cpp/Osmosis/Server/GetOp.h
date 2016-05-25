#ifndef __OSMOSIS_SERVER_GET_OP_H__
#define __OSMOSIS_SERVER_GET_OP_H__

#include "Osmosis/TCPSocket.h"
#include "Osmosis/ObjectStore/Store.h"
#include "Osmosis/Stream/FileToSocket.h"
#include "Osmosis/Stream/BufferToSocket.h"

namespace Osmosis {
namespace Server
{

class GetOp
{
public:
	GetOp( TCPSocket & socket, const ObjectStore::Store & store ) :
		_socket( socket ),
		_store( store )
	{}

	void go()
	{
		BACKTRACE_BEGIN
		Hash hash( _socket.receiveAll< struct Tongue::Hash >() );
		if ( not _store.exists( hash ) )
			THROW( Error, "Hash " << hash << " is not in object store, can not get" );
#ifdef DEBUG
		if ( not CalculateHash::verify( _store.filenameForExisting( hash ), hash ) ) {
			TRACE_ERROR( "Stored object " << hash << " does not match its hash" );
			_store.verifyOrDestroy( hash );
			std::string message = "Malformed object found in object store";
			Stream::BufferToSocket transfer( message.c_str(), message.size(), _socket );
			transfer.transfer();
			return;
		}
#endif // DEBUG
		Stream::FileToSocket transfer( _store.filenameForExisting( hash ).string().c_str(), _socket );
		transfer.transfer();
		BACKTRACE_END
	}

private:
	TCPSocket & _socket;
	const ObjectStore::Store & _store;

	GetOp( const GetOp & rhs ) = delete;
	GetOp & operator= ( const GetOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_GET_OP_H__
