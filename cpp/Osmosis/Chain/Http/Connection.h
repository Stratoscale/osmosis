#ifndef __OSMOSIS_CHAIN_HTTP_CONNECTION_H__
#define __OSMOSIS_CHAIN_HTTP_CONNECTION_H__

#include <boost/network/include/http/client.hpp>
#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"

namespace Osmosis {
namespace Chain {
namespace Http
{

class Connection : public ObjectStoreConnectionInterface
{
public:
	Connection( const std::string & url ):
		_url( url )
	{}

	void putString( const std::string & blob, const Hash & hash ) override
	{
		THROW( Error, "'http://' object store does not support the putString operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	std::string getString( const Hash & hash ) override
	{
		boost::network::http::client::request request( hashUrl( hash ) );
		boost::network::http::client::response response = _client.get( request );
		std::ostringstream out;
		out << boost::network::http::body( response );
		return std::move( out.str() );
	}

	void putFile( const boost::filesystem::path & path, const Hash & hash ) override
	{
		THROW( Error, "'http://' object store does not support the putFile operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	void getFile( const boost::filesystem::path & path, const Hash & hash ) override
	{
		boost::network::http::client::request request( hashUrl( hash ) );
		boost::network::http::client::response response = _client.get( request );
		std::ofstream out( path.string(), std::ios::out | std::ios::binary );
		out << boost::network::http::body( response );
	}

	bool exists( const Hash & hash ) override
	{
		boost::network::http::client::request request( hashUrl( hash ) );
		boost::network::http::client::response response = _client.head( request );
		auto status = boost::network::http::status( response );
		return status == 200;
	}

	void verify( const Hash & hash ) override
	{}

	void eraseLabel( const std::string & label ) override
	{
		THROW( Error, "'http://' object store does not support the eraseLabel operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	void purge() override
	{
		THROW( Error, "'http://' object store does not support the purge operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	void setLabel( const Hash & hash, const std::string & label ) override
	{
		THROW( Error, "'http://' object store does not support the setLabel operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	Hash getLabel( const std::string & label ) override
	{
		std::string url = _url + "/" + ObjectStore::DirectoryNames::LABELS + "/" + label;
		boost::network::http::client::request request( url );
		boost::network::http::client::response response = _client.get( request );
		std::ostringstream out;
		out << boost::network::http::body( response );
		return Hash::fromHex( out.str() );
	}

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo ) override
	{
		THROW( Error, "'http://' object store does not support the renameLabel operation "
				"(this kind of object store can only be used for checkout operations "
				"as the last in the chain)" );
	}

	std::list< std::string > listLabels( const std::string & regex ) override
	{
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
	}

private:
	const std::string             _url;
	boost::network::http::client  _client;

	std::string hashUrl( const Hash & hash )
	{
		return _url + "/" + hash.relativeFilename().string();
	}

	Connection( const Connection & rhs ) = delete;
	Connection & operator= ( const Connection & rhs ) = delete;
};

} // namespace Http
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_HTTP_CONNECTION_H__
