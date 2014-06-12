#ifndef __OSMOSIS_SERVER_RENAME_LABEL_OP_H__
#define __OSMOSIS_SERVER_RENAME_LABEL_OP_H__

namespace Osmosis {
namespace Server
{

class RenameLabelOp
{
public:
	RenameLabelOp(  TCPSocket &            socket,
			ObjectStore::Labels &  labels ) :
		_socket( socket ),
		_labels( labels )
	{}

	void go()
	{
		std::string currentLabel( ReceiveLabel( _socket ).label() );
		if ( not _labels.exists( currentLabel ) )
			THROW( Error, "Label '" << currentLabel << "' does not exist, can not rename" );
		std::string renameTo( ReceiveLabel( _socket ).label() );
		if ( _labels.exists( renameTo ) )
			THROW( Error, "Label '" << renameTo << "' already exists" );
		_labels.rename( currentLabel, renameTo );
		Stream::AckOps( _socket ).sendAck();
	}

private:
	TCPSocket &            _socket;
	ObjectStore::Labels &  _labels;

	RenameLabelOp( const RenameLabelOp & rhs ) = delete;
	RenameLabelOp & operator= ( const RenameLabelOp & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_RENAME_LABEL_OP_H__
