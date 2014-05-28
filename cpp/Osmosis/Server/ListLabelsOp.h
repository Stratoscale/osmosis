#ifndef __OSMOSIS_SERVER_LIST_LABELS_OP_H__
#define __OSMOSIS_SERVER_LIST_LABELS_OP_H__

#include "Osmosis/ObjectStore/Labels.h"

namespace Osmosis {
namespace Server
{

class ListLabelsOp
{
public:
	ListLabelsOp( TCPSocket & socket, const ObjectStore::Labels & labels ) :
		_socket( socket ),
		_labels( labels ),
		_outgoing( socket )
	{}

	void go()
	{
		char regexBuffer[ 1024 ];
		auto raw = _socket.recieveAll< struct Tongue::Label >();
		if ( raw.length > sizeof( regexBuffer ) )
			THROW( Error, "Label glob expression maximum size of " << sizeof( regexBuffer ) << " exceeded" );
		_socket.recieveAll( regexBuffer, raw.length );
		std::string regex( regexBuffer, raw.length );
		for ( auto i = _labels.list( regex ); not i.done(); i.next() ) {
			std::string label = * i;
			_outgoing.send( 0, label.c_str(), label.size() );
		}
		_outgoing.sendEOF();
	}

private:
	TCPSocket &                  _socket;
	const ObjectStore::Labels &  _labels;
	Stream::Outgoing             _outgoing;

	ListLabelsOp( const ListLabelsOp & rhs ) = delete;
	ListLabelsOp & operator= ( const ListLabelsOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_LIST_LABELS_OP_H__
