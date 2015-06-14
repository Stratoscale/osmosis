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
		BACKTRACE_BEGIN
		_connection->eraseLabel( label );
		BACKTRACE_END
	}

	void renameLabel( const std::string & currentLabel, const std::string & renameLabelTo )
	{
		BACKTRACE_BEGIN
		_connection->renameLabel( currentLabel, renameLabelTo );
		BACKTRACE_END
	}

	std::list< std::string > listLabels( const std::string & regex )
	{
		BACKTRACE_BEGIN
		return std::move( _connection->listLabels( regex ) );
		BACKTRACE_END
	}

private:
	std::unique_ptr< Chain::ObjectStoreConnectionInterface > _connection;

	LabelOps( const LabelOps & rhs ) = delete;
	LabelOps & operator= ( const LabelOps & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_LABEL_OPS_H__
