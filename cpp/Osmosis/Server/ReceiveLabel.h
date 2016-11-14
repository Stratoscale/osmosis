#ifndef __OSMOSIS_SERVER_RECEIVE_LABEL_H__
#define __OSMOSIS_SERVER_RECEIVE_LABEL_H__

namespace Osmosis {
namespace Server
{

class ReceiveLabel
{
public:
	ReceiveLabel( TCPSocket & socket )
	{
		char buffer[ 1024 ];
		auto raw = socket.receiveAll< struct Tongue::Label >();
		if ( raw.length > sizeof( buffer ) )
			THROW( Error, "Label maximum size of " << sizeof( buffer ) << " exceeded" );
		socket.receiveAll( buffer, raw.length );
		_label = std::move( std::string( buffer, raw.length ) );
	}

	const std::string & label() const
	{
		return _label;
	}

private:
	std::string _label;

	ReceiveLabel( const ReceiveLabel & rhs ) = delete;
	ReceiveLabel & operator= ( const ReceiveLabel & rhs ) = delete;
};

} // namespace Server
} // namespace Osmosis

#endif // __OSMOSIS_SERVER_RECEIVE_LABEL_H__
