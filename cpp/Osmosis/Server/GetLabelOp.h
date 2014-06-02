#ifndef __OSMOSIS_SERVER_GET_LABEL_OP_H__
#define __OSMOSIS_SERVER_GET_LABEL_OP_H__

#include "Osmosis/ObjectStore/Labels.h"
#include "Osmosis/Server/ReceiveLabel.h"

namespace Osmosis {
namespace Server
{

class GetLabelOp
{
public:
	GetLabelOp( TCPSocket & socket, const ObjectStore::Labels & labels ) :
		_socket( socket ),
		_labels( labels )
	{}

	void go()
	{
		std::string label( ReceiveLabel( _socket ).label() );
		if ( not _labels.exists( label ) )
			THROW( Error, "Label '" << label << "' does not exist, can not get" );
		Hash hash = _labels.readLabel( label );
		_socket.sendAll( hash.raw() );
	}

private:
	TCPSocket &                  _socket;
	const ObjectStore::Labels &  _labels; 

	GetLabelOp( const GetLabelOp & rhs ) = delete;
	GetLabelOp & operator= ( const GetLabelOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_GET_LABEL_OP_H__
