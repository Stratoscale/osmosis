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
		BACKTRACE_BEGIN
		std::string regex( ReceiveLabel( _socket ).label() );
		for ( auto i = _labels.list( regex ); not i.done(); i.next() ) {
			std::string label = * i;
			_outgoing.send( 0, label.c_str(), label.size() );
		}
		_outgoing.sendEOF();
		BACKTRACE_END
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
