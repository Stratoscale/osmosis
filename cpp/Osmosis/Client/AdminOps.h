#ifndef __OSMOSIS_CLIENT_ADMIN_OPS_H__
#define __OSMOSIS_CLIENT_ADMIN_OPS_H__

namespace Osmosis {
namespace Client
{

class AdminOps
{
public:
	AdminOps( Chain::ObjectStoreInterface & objectStore ) :
		_connection( objectStore.connect() )
	{}

	void purge()
	{
		_connection->purge();
	}

private:
	std::unique_ptr< Chain::ObjectStoreConnectionInterface > _connection;

	AdminOps( const AdminOps & rhs ) = delete;
	AdminOps & operator= ( const AdminOps & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_ADMIN_OPS_H__
