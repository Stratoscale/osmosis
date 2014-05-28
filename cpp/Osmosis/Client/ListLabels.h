#ifndef __OSMOSIS_CLIENT_LIST_LABELS_H__
#define __OSMOSIS_CLIENT_LIST_LABELS_H__

namespace Osmosis {
namespace Client
{

class ListLabels
{
public:
	ListLabels( const std::string & labelRegex, const std::string & hostname, unsigned short port ) :
		_labelRegex( labelRegex ),
		_connection( hostname, port ),
		_labelOps( _connection.socket() )
	{}

	const std::list< std::string > & result() { return _result; }

	void go()
	{
		_labelOps.sendLabelCommand( _labelRegex, Tongue::Opcode::LIST_LABELS );
		for ( Stream::Incoming incoming( _connection.socket() ); not incoming.done(); incoming.next() ) {
			std::string label( reinterpret_cast< const char * >( incoming.buffer() ), incoming.bufferLength() );
			_result.push_back( std::move( label ) );
		}
	}

private:
	const std::string         _labelRegex;
	Connect                   _connection;
	LabelOps                  _labelOps;
	std::list< std::string >  _result;

	ListLabels( const ListLabels & rhs ) = delete;
	ListLabels & operator= ( const ListLabels & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_LIST_LABELS_H__
