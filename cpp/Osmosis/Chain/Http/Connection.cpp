#include <fstream>
#include <boost/network/include/http/client.hpp>
#include "Osmosis/Chain/Http/Connection.h"
#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"
#include "Osmosis/ObjectStore/DirectoryNames.h"

namespace Osmosis {
namespace Chain {
namespace Http
{

Connection::Connection( const std::string & url ):
	_url( url )
{}

void Connection::putString( const std::string & blob, const Hash & hash )
{
	THROW( Error, "'http://' object store does not support the putString operation "
			"(this kind of object store can only be used for checkout operations "
			"as the last in the chain)" );
}

std::string Connection::getString( const Hash & hash )
{
	BACKTRACE_BEGIN
	boost::network::http::client::request request( hashUrl( hash ) );
	boost::network::http::client::response response = _client.get( request );
	std::ostringstream out;
	out << boost::network::http::body( response );
	return std::move( out.str() );
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Connection::putFile( const boost::filesystem::path & path, const Hash & hash )
{
	THROW( Error, "'http://' object store does not support the putFile operation "
			"(this kind of object store can only be used for checkout operations "
			"as the last in the chain)" );
}

void Connection::getFile( const boost::filesystem::path & path, const Hash & hash )
{
	BACKTRACE_BEGIN
	boost::network::http::client::request request( hashUrl( hash ) );
	boost::network::http::client::response response = _client.get( request );
	std::ofstream out( path.string(), std::ios::out | std::ios::binary );
	out << boost::network::http::body( response );
	BACKTRACE_END_VERBOSE( "Path " << path << " Hash " << hash );
}

bool Connection::exists( const Hash & hash )
{
	BACKTRACE_BEGIN
	boost::network::http::client::request request( hashUrl( hash ) );
	boost::network::http::client::response response = _client.head( request );
	auto status = boost::network::http::status( response );
	return status == 200;
	BACKTRACE_END_VERBOSE( "Hash " << hash );
}

void Connection::verify( const Hash & hash )
{}

void Connection::eraseLabel( const std::string & label )
{
	THROW( Error, "'http://' object store does not support the eraseLabel operation "
			"(this kind of object store can only be used for checkout operations "
			"as the last in the chain)" );
}

void Connection::setLabel( const Hash & hash, const std::string & label )
{
	THROW( Error, "'http://' object store does not support the setLabel operation "
			"(this kind of object store can only be used for checkout operations "
			"as the last in the chain)" );
}

Hash Connection::getLabel( const std::string & label )
{
	BACKTRACE_BEGIN
	std::string url = _url + "/" + ObjectStore::DirectoryNames::LABELS + "/" + label;
	boost::network::http::client::request request( url );
	boost::network::http::client::response response = _client.get( request );
	std::ostringstream out;
	out << boost::network::http::body( response );
	return Hash( out.str() );
	BACKTRACE_END_VERBOSE( "Label " << label );
}

void Connection::renameLabel( const std::string & currentLabel, const std::string & renameLabelTo )
{
	THROW( Error, "'http://' object store does not support the renameLabel operation "
			"(this kind of object store can only be used for checkout operations "
			"as the last in the chain)" );
}

std::list< std::string > Connection::listLabels( const std::string & regex )
{
	BACKTRACE_BEGIN
	if ( regex[ 0 ] != '^' and regex[ regex.size() - 1 ] != '$' )
		THROW( Error, "'http://' object store does not support the listLabels operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	std::string label = regex.substr( 1, regex.size() - 2 );
	std::string url = _url + "/" + ObjectStore::DirectoryNames::LABELS + "/" + label;
	boost::network::http::client::request request( url );
	boost::network::http::client::response response = _client.head( request );
	std::list< std::string > result;
	auto status = boost::network::http::status( response );
	if ( status == 200 )
		result.push_back( label );
	return result;
	BACKTRACE_END_VERBOSE( "Regex " << regex );
}

std::string Connection::hashUrl( const Hash & hash )
{
	return _url + "/" + hash.relativeFilename().string();
}

} // namespace Http
} // namespace Chain
} // namespace Osmosis
