#ifndef __OSMOSIS_CLIENT_LABEL_OPS_H__
#define __OSMOSIS_CLIENT_LABEL_OPS_H__

namespace Osmosis {
namespace Client
{

class LabelOps
{
public:
	LabelOps( Chain::ObjectStoreInterface & objectStore ) :
		_connection( objectStore.connect() )
	{}

	void eraseLabel( const std::string & label )
	{
		_connection->eraseLabel( label );
	}

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo )
	{
		_connection->renameLabel( currentLabel, renameLabelTo );
	}

	std::list< std::string > listLabels( const std::string & regex )
	{
		return std::move( _connection->listLabels( regex ) );
	}

private:
	std::unique_ptr< Chain::ObjectStoreConnectionInterface > _connection;

	LabelOps( const LabelOps & rhs ) = delete;
	LabelOps & operator= ( const LabelOps & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_LABEL_OPS_H__
