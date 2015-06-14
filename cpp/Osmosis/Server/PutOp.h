#ifndef __OSMOSIS_SERVER_PUT_OP_H__
#define __OSMOSIS_SERVER_PUT_OP_H__

#include "Osmosis/ObjectStore/Drafts.h"
#include "Osmosis/Stream/SocketToFile.h"

namespace Osmosis {
namespace Server
{

class PutOp
{
public:
	PutOp( TCPSocket & socket, ObjectStore::Store & store, ObjectStore::Drafts & drafts ) :
		_socket( socket ),
		_store( store ),
		_drafts( drafts )
	{}

	void go()
	{
		BACKTRACE_BEGIN
		Hash hash( _socket.recieveAll< struct Tongue::Hash >() );
		if ( _store.exists( hash ) )
			THROW( Error, "Will not store an object that already exists: " << hash );
		auto filename = _drafts.allocateFilename();
		Stream::SocketToFile transfer( _socket, filename.string().c_str() );
		transfer.transfer();
		if ( not CalculateHash::verify( filename, hash ) )
			THROW( Error, "Incoming draft did not match hash: " << hash );
		_store.putExistingFile( hash, filename );
		Stream::AckOps( _socket ).sendAck();
		BACKTRACE_END
	}

private:
	TCPSocket & _socket;
	ObjectStore::Store & _store;
	ObjectStore::Drafts & _drafts;

	PutOp( const PutOp & rhs ) = delete;
	PutOp & operator= ( const PutOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_PUT_OP_H__
