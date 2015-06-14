#ifndef __OSMOSIS_SERVER_SET_LABEL_OP_H__
#define __OSMOSIS_SERVER_SET_LABEL_OP_H__

namespace Osmosis {
namespace Server
{

class SetLabelOp
{
public:
	SetLabelOp( TCPSocket & socket, ObjectStore::Labels & labels ) :
		_socket( socket ),
		_labels( labels )
	{}

	void go()
	{
		BACKTRACE_BEGIN
		std::string label( ReceiveLabel( _socket ).label() );
		if ( _labels.exists( label ) )
			THROW( Error, "Label '" << label << "' already exists, can not set" );
		auto rawHash = _socket.recieveAll< struct Tongue::Hash >();
		_labels.label( Hash( rawHash ), label );
		Stream::AckOps( _socket ).sendAck();
		BACKTRACE_END
	}

private:
	TCPSocket &            _socket;
	ObjectStore::Labels &  _labels; 

	SetLabelOp( const SetLabelOp & rhs ) = delete;
	SetLabelOp & operator= ( const SetLabelOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_SET_LABEL_OP_H__
