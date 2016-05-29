#include <string>
#include <Osmosis/Chain/Remote/Connection.h>
#include <Osmosis/CalculateHash.h>
#include <Osmosis/Stream/BufferToSocket.h>
#include <Osmosis/Stream/FileToSocket.h>
#include <Osmosis/Stream/SocketToBuffer.h>
#include <Osmosis/Stream/SocketToFile.h>

namespace Osmosis {
namespace Chain {
namespace Remote
{

Connection::Connection( const std::string & hostname, unsigned short port ) :
	_connection( hostname, port ),
	_labelOps( _connection.socket() )
{}

void Connection::putString( const std::string & blob, const Hash & hash )
{
	BACKTRACE_BEGIN
	try {
		ASSERT( CalculateHash::verify( blob.c_str(), blob.size(), hash ) );
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::PUT ) };
		_connection.socket().sendAllConcated( header, hash.raw() );
		Stream::BufferToSocket transfer( blob.c_str(), blob.size(), _connection.socket() );
		transfer.transfer();
		Stream::AckOps( _connection.socket() ).wait( "Put blob" );
	} catch (...) {
		TRACE_ERROR( "While transferring blob (possibly a dirList)" );
		throw;
	}
	BACKTRACE_END_VERBOSE( "Hash " << hash );
	TRACE_DEBUG( "Transferred blob (possibly a dirList)" );
}

std::string Connection::getString( const Hash & hash )
{
	BACKTRACE_BEGIN
	struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
	_connection.socket().sendAllConcated( header, hash.raw() );
	Stream::SocketToBuffer transfer( _connection.socket() );
	transfer.transfer();
	std::string blob = transfer.data();
	if ( not CalculateHash::verify( blob.c_str(), blob.size(), hash ) )
		THROW( Error, "Blob hash did not match contents: " << hash << " (size: " << blob.size() << ")" );
	return std::move( blob );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Connection::putFile( const boost::filesystem::path & path, const Hash & hash )
{
	BACKTRACE_BEGIN
	try {
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::PUT ) };
		_connection.socket().sendAllConcated( header, hash.raw() );
		Stream::FileToSocket transfer( path.string().c_str(), _connection.socket() );
		transfer.transfer();
		Stream::AckOps( _connection.socket() ).wait( "Put object" );
	} catch (...) {
		TRACE_ERROR( "While transferring file " << path );
		throw;
	}
	BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
	TRACE_DEBUG( "Transferred file " << path );
}

void Connection::getFile( const boost::filesystem::path & path, const Hash & hash )
{
	BACKTRACE_BEGIN
	struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
	_connection.socket().sendAllConcated( header, hash.raw() );
	Stream::SocketToFile transfer( _connection.socket(), path.string().c_str() );
	transfer.transfer();
	BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
}

bool Connection::exists( const Hash & hash )
{
	BACKTRACE_BEGIN
	struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::IS_EXISTS ) };
	_connection.socket().sendAllConcated( header, hash.raw() );
	auto response = _connection.socket().receiveAll< struct Tongue::IsExistsResponse >();
	if ( response.response != static_cast< unsigned char >( Tongue::IsExists::YES ) and
			response.response != static_cast< unsigned char > ( Tongue::IsExists::NO ) )
		THROW( Error, "Invalid response from server for an exists query: " << static_cast< unsigned >( response.response ) );
	return response.response == static_cast< unsigned char >( Tongue::IsExists::YES );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Connection::verify( const Hash & hash )
{
	BACKTRACE_BEGIN
	struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::VERIFY ) };
	_connection.socket().sendAllConcated( header, hash.raw() );
	Stream::AckOps( _connection.socket() ).wait( "Verify" );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Connection::eraseLabel( const std::string & label )
{
	_labelOps.erase( label );
}

void Connection::setLabel( const Hash & hash, const std::string & label )
{
	_labelOps.set( hash, label );
}

Hash Connection::getLabel( const std::string & label )
{
	return _labelOps.get( label );
}

void Connection::renameLabel( const std::string & currentLabel, const std::string & renameLabelTo )
{
	_labelOps.rename( currentLabel, renameLabelTo );
}

std::list< std::string > Connection::listLabels( const std::string & regex )
{
	return std::move( _labelOps.list( regex ) );
}

} // namespace Remote
} // namespace Chain
} // namespace Osmosis
