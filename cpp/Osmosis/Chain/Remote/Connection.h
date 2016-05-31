#ifndef __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__
#define __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__

#include <boost/filesystem/path.hpp>
#include "Osmosis/Chain/ObjectStoreConnectionInterface.h"
#include "Osmosis/Chain/Remote/LabelOps.h"
#include "Osmosis/TCPConnection.h"
#include "Osmosis/Hash.h"

namespace Osmosis {
namespace Chain {
namespace Remote
{

class Connection : public ObjectStoreConnectionInterface
{
public:
	Connection( const std::string & hostname, unsigned short port );

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
	TCPConnection  _connection;
	LabelOps       _labelOps;

	Connection( const Connection & rhs ) = delete;

	Connection & operator= ( const Connection & rhs ) = delete;
};

} // namespace Remote
} // namespace Chain
} // namespace Osmosis

#endif // __OSMOSIS_CHAIN_REMOTE_CONNECTION_H__
