#ifndef __OSMOSIS_SERVER_ERASE_LABEL_OP_H__
#define __OSMOSIS_SERVER_ERASE_LABEL_OP_H__

#include "Osmosis/ObjectStore/Purge.h"

namespace Osmosis {
namespace Server
{

class EraseLabelOp
{
public:
	EraseLabelOp(   TCPSocket &            socket,
			ObjectStore::Store &   store,
			ObjectStore::Labels &  labels ) :
		_socket( socket ),
		_store( store ),
		_labels( labels )
	{}

	void go()
	{
		BACKTRACE_BEGIN
		std::string label( ReceiveLabel( _socket ).label() );
		if ( not _labels.exists( label ) )
			THROW( Error, "Label '" << label << "' does not exist, can not erase" );
		_labels.erase( label );
		Stream::AckOps( _socket ).sendAck();
		BACKTRACE_END
	}

private:
	TCPSocket &            _socket;
	ObjectStore::Store &   _store;
	ObjectStore::Labels &  _labels;

	EraseLabelOp( const EraseLabelOp & rhs ) = delete;
	EraseLabelOp & operator= ( const EraseLabelOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_ERASE_LABEL_OP_H__
