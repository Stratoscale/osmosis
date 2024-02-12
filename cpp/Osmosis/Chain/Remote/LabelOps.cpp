#include <Osmosis/Chain/Remote/LabelOps.h>
#include <Osmosis/TCPSocket.h>
#include <Osmosis/Stream/AckOps.h>
#include <Osmosis/Stream/Incoming.h>

namespace Osmosis {
namespace Chain {
namespace Remote
{

LabelOps::LabelOps( TCPSocket & socket ):
	_socket( socket )
{}

void LabelOps::set( const Hash & hash, const std::string & label )
{
	BACKTRACE_BEGIN
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
	Stream::AckOps( _socket ).wait( "setting label" );
	BACKTRACE_END_VERBOSE( "Hash " << hash << " Label " << label );
}

Hash LabelOps::get( const std::string & label )
{
	BACKTRACE_BEGIN
	sendLabelCommand( label, Tongue::Opcode::GET_LABEL );
	try {
		Hash hash( _socket.receiveAll< struct Tongue::Hash >() );
		return hash;
	} catch ( boost::system::system_error & e ) {
		if ( e.code() == boost::asio::error::eof )
			THROW( Error, "Peer terminated connection instead of sending ack, when getting label '" <<
					label << " hash: maybe the label does not exist?" );
		throw;
	}
	BACKTRACE_END_VERBOSE( "Label " << label );
}

std::list< std::string > LabelOps::list( const std::string regex )
{
	BACKTRACE_BEGIN
	sendLabelCommand( regex, Tongue::Opcode::LIST_LABELS );
	std::list< std::string > result;
	for ( Stream::Incoming incoming( _socket ); not incoming.done(); incoming.next() ) {
		std::string label( reinterpret_cast< const char * >( incoming.buffer() ), incoming.bufferLength() );
		result.push_back( std::move( label ) );
	}
	return result;
	BACKTRACE_END_VERBOSE( "Regex " << regex );
}

void LabelOps::rename( const std::string & from, const std::string & to )
{
	BACKTRACE_BEGIN
	sendRenameCommand( from, to );
	Stream::AckOps( _socket ).wait( "Rename operation" );
	BACKTRACE_END_VERBOSE( "From " << from << " To " << to );
}

void LabelOps::erase( const std::string & label )
{
	BACKTRACE_BEGIN
	sendLabelCommand( label, Tongue::Opcode::ERASE_LABEL );
	Stream::AckOps( _socket ).wait( "Erased label" );
	BACKTRACE_END_VERBOSE( "Label " << label );
}

void LabelOps::sendLabelCommand( const std::string & label, Tongue::Opcode opcode )
{
	BACKTRACE_BEGIN
	unsigned char buffer[ 1024 ];
	struct Tongue::Header * header = reinterpret_cast< struct Tongue::Header * >( buffer );
	struct Tongue::Label * labelHeader = reinterpret_cast< struct Tongue::Label * >( header + 1 );
	unsigned char * labelText = reinterpret_cast< unsigned char * >( labelHeader + 1 );
	size_t size = reinterpret_cast< unsigned char * >( labelText + label.size() ) - buffer;
	if ( size > sizeof( buffer ) ) 
		THROW( Error, "Label too long" );
	header->opcode = static_cast< unsigned char >( opcode );
	ASSERT( label.size() < ( 1 << 16 ) );
	labelHeader->length = static_cast< unsigned short >( label.size() );
	memcpy( labelText, label.c_str(), label.size() );
	_socket.sendAll( buffer, size );
	BACKTRACE_END_VERBOSE( "Label " << label << " Opcode " << static_cast< unsigned >( opcode ) );
}

void LabelOps::sendRenameCommand( const std::string & from, const std::string & to )
{
	unsigned char buffer[ 2048 ];
	struct Tongue::Header * header = reinterpret_cast< struct Tongue::Header * >( buffer );
	struct Tongue::Label * label1Header = reinterpret_cast< struct Tongue::Label * >( header + 1 );
	unsigned char * label1Text = reinterpret_cast< unsigned char * >( label1Header + 1 );
	struct Tongue::Label * label2Header = reinterpret_cast< struct Tongue::Label * >( label1Text + from.size() );
	unsigned char * label2Text = reinterpret_cast< unsigned char * >( label2Header + 1 );
	size_t size = reinterpret_cast< unsigned char * >( label2Text + to.size() ) - buffer;
	if ( size > sizeof( buffer ) ) 
		THROW( Error, "Label too long" );

	header->opcode = static_cast< unsigned char >( Tongue::Opcode::RENAME_LABEL );

	ASSERT( from.size() < ( 1 << 16 ) );
	label1Header->length = static_cast< unsigned short >( from.size() );
	memcpy( label1Text, from.c_str(), from.size() );

	ASSERT( to.size() < ( 1 << 16 ) );
	label2Header->length = static_cast< unsigned short >( to.size() );
	memcpy( label2Text, to.c_str(), to.size() );

	_socket.sendAll( buffer, size );
}

} // namespace Remote
} // namespace Chain
} // namespace Osmosis
