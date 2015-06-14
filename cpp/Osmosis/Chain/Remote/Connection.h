#ifndef __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__
#define __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__

#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"
#include "Osmosis/Chain/Remote/LabelOps.h"
#include "Osmosis/TCPConnection.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

class Connection : public ObjectStoreConnectionInterface
{
public:
	Connection( const std::string & hostname, unsigned short port ) :
		_connection( hostname, port ),
		_labelOps( _connection.socket() )
	{}

	void putString( const std::string & blob, const Hash & hash ) override
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

	std::string getString( const Hash & hash ) override
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

	void putFile( const boost::filesystem::path & path, const Hash & hash ) override
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

	void getFile( const boost::filesystem::path & path, const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
		_connection.socket().sendAllConcated( header, hash.raw() );
		Stream::SocketToFile transfer( _connection.socket(), path.string().c_str() );
		transfer.transfer();
		BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
	}

	bool exists( const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::IS_EXISTS ) };
		_connection.socket().sendAllConcated( header, hash.raw() );
		auto response = _connection.socket().recieveAll< struct Tongue::IsExistsResponse >();
		if ( response.response != static_cast< unsigned char >( Tongue::IsExists::YES ) and
				response.response != static_cast< unsigned char > ( Tongue::IsExists::NO ) )
			THROW( Error, "Invalid response from server for an exists query: " << static_cast< unsigned >( response.response ) );
		return response.response == static_cast< unsigned char >( Tongue::IsExists::YES );
		BACKTRACE_END_VERBOSE( "Hash " << hash );
	}

	void verify( const Hash & hash ) override
	{
		BACKTRACE_BEGIN
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::VERIFY ) };
		_connection.socket().sendAllConcated( header, hash.raw() );
		Stream::AckOps( _connection.socket() ).wait( "Verify" );
		BACKTRACE_END_VERBOSE( "Hash " << hash );
	}

	void eraseLabel( const std::string & label ) override
	{
		_labelOps.erase( label );
	}

	void setLabel( const Hash & hash, const std::string & label ) override
	{
		_labelOps.set( hash, label );
	}

	Hash getLabel( const std::string & label ) override
	{
		return _labelOps.get( label );
	}

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo ) override
	{
		_labelOps.rename( currentLabel, renameLabelTo );
	}

	std::list< std::string > listLabels( const std::string & regex ) override
	{
		return std::move( _labelOps.list( regex ) );
	}

private:
	TCPConnection  _connection;
	LabelOps       _labelOps;

	Connection( const Connection & rhs ) = delete;
	Connection & operator= ( const Connection & rhs ) = delete;
};

} // namespace Remote
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__
