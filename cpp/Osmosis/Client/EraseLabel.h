#ifndef __OSMOSIS_CLIENT_ERASE_LABEL_H__
#define __OSMOSIS_CLIENT_ERASE_LABEL_H__

namespace Osmosis {
namespace Client
{

class EraseLabel
{
public:
	EraseLabel( const std::string & label, const std::string & hostname, unsigned short port ) :
		_label( label ),
		_connection( hostname, port ),
		_labelOps( _connection.socket() )
	{}

	void go()
	{
		_labelOps.sendLabelCommand( _label, Tongue::Opcode::ERASE_LABEL );
		Stream::AckOps( _connection.socket() ).wait( "Purged objects" );
	}

private:
	const std::string         _label;
	Connect                   _connection;
	LabelOps                  _labelOps;

	EraseLabel( const EraseLabel & rhs ) = delete;
	EraseLabel & operator= ( const EraseLabel & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_ERASE_LABEL_H__
