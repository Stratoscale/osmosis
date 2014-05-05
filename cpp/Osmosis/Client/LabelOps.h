#ifndef __OSMOSIS_CLIENT_LABEL_OPS_H__
#define __OSMOSIS_CLIENT_LABEL_OPS_H__

namespace Osmosis {
namespace Client
{

class LabelOps
{
public:
	LabelOps( TCPSocket & socket ):
		_socket( socket ),
		_waitForAck( socket )
	{}

	void set( const Hash & hash, const std::string & label )
	{
		unsigned char buffer[ 1024 ];
		struct Tongue::Header * header = reinterpret_cast< struct Tongue::Header * >( buffer );
		struct Tongue::Label * labelHeader = reinterpret_cast< struct Tongue::Label * >( header + 1 );
		unsigned char * labelText = reinterpret_cast< unsigned char * >( labelHeader + 1 );
		struct Tongue::Hash * rawHash = reinterpret_cast< struct Tongue::Hash * >( labelText + label.size() );
		if ( reinterpret_cast< unsigned char * >( rawHash + 1 ) > buffer + sizeof( buffer ) ) 
			THROW( Error, "Label too long" );
		header->opcode = static_cast< unsigned char >( Tongue::Opcode::SET_LABEL );
		ASSERT( label.size() < ( 1 << 16 ) );
		labelHeader->length = static_cast< unsigned short >( label.size() );
		memcpy( labelText, label.c_str(), label.size() );
		* rawHash = hash.raw();
		size_t size = reinterpret_cast< unsigned char * >( rawHash + 1 ) - buffer;
		_socket.sendAll( buffer, size );
		_waitForAck.wait( "setting label" );
	}

	Hash get( const std::string & label )
	{
		unsigned char buffer[ 1024 ];
		struct Tongue::Header * header = reinterpret_cast< struct Tongue::Header * >( buffer );
		struct Tongue::Label * labelHeader = reinterpret_cast< struct Tongue::Label * >( header + 1 );
		unsigned char * labelText = reinterpret_cast< unsigned char * >( labelHeader + 1 );
		size_t size = reinterpret_cast< unsigned char * >( labelText + label.size() ) - buffer;
		if ( size > sizeof( buffer ) ) 
			THROW( Error, "Label too long" );
		header->opcode = static_cast< unsigned char >( Tongue::Opcode::GET_LABEL );
		ASSERT( label.size() < ( 1 << 16 ) );
		labelHeader->length = static_cast< unsigned short >( label.size() );
		memcpy( labelText, label.c_str(), label.size() );
		_socket.sendAll( buffer, size );
		Hash hash( _socket.recieveAll< struct Tongue::Hash >() );
		return hash;
	}

private:
	TCPSocket &  _socket; 
	Stream::WaitForAck _waitForAck;

	LabelOps( const LabelOps & rhs ) = delete;
	LabelOps & operator= ( const LabelOps & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_LABEL_OPS_H__
