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
	Connection( const std::string & url );

	void putString( const std::string & blob, const Hash & hash ) override;

	std::string getString( const Hash & hash ) override;

	void putFile( const boost::filesystem::path & path, const Hash & hash ) override;

	void getFile( const boost::filesystem::path & path, const Hash & hash ) override;

	bool exists( const Hash & hash ) override;

	void verify( const Hash & hash ) override;

	void eraseLabel( const std::string & label ) override;

	void setLabel( const Hash & hash, const std::string & label ) override;

	Hash getLabel( const std::string & label ) override;

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo ) override;

	std::list< std::string > listLabels( const std::string & regex ) override;

private:
	const std::string             _url;
	boost::network::http::client  _client;

	std::string hashUrl( const Hash & hash );

	Connection( const Connection & rhs ) = delete;
	Connection & operator= ( const Connection & rhs ) = delete;
};

} // namespace Http
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_HTTP_CONNECTION_H__
