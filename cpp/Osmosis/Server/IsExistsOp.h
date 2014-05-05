#ifndef __OSMOSIS_SERVER_IS_EXISTS_OP_H__
#define __OSMOSIS_SERVER_IS_EXISTS_OP_H__

namespace Osmosis {
namespace Server
{

class IsExistsOp
{
public:
	IsExistsOp( TCPSocket & socket, const ObjectStore::Store & store ) :
		_socket( socket ),
		_store( store )
	{}

	void go()
	{
		Hash hash( _socket.recieveAll< struct Tongue::Hash >() );
		struct Tongue::IsExistsResponse response = { static_cast< unsigned char >(
			_store.exists( hash ) ? Tongue::IsExists::YES : Tongue::IsExists::NO ) };
		_socket.sendAll( response );
	}

private:
	TCPSocket & _socket;
	const ObjectStore::Store & _store;

	IsExistsOp( const IsExistsOp & rhs ) = delete;
	IsExistsOp & operator= ( const IsExistsOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_IS_EXISTS_OP_H__
