#ifndef __OSMOSIS_OBJECT_STORE_DRAFTS_H__
#define __OSMOSIS_OBJECT_STORE_DRAFTS_H__

#include <mutex>

namespace Osmosis {
namespace ObjectStore
{

class Drafts
{
public:
	Drafts( const boost::filesystem::path & rootPath ) :
		_rootPath( rootPath ),
		_draftsPath( rootPath / DirectoryNames::DRAFTS ),
		_counter( 0 )
	{
		if ( not boost::filesystem::is_directory( _draftsPath ) )
			boost::filesystem::create_directories( _draftsPath );
		cleanUp();
	}

	void cleanUp()
	{
		for ( auto i = boost::filesystem::directory_iterator( _draftsPath );
				i != boost::filesystem::directory_iterator(); ++ i ) {
			bool removed = boost::filesystem::remove( i->path() );
			if ( not removed )
				TRACE_ERROR( "File was removed under my feet " << i->path() );
		}
	}

	void eraseDirectory()
	{
		cleanUp();
		bool removed = boost::filesystem::remove( _draftsPath );
		if ( not removed )
			TRACE_ERROR( "File was removed under my feet " << _draftsPath );
	}

	boost::filesystem::path allocateFilename()
	{
		size_t value;
		{
			std::lock_guard< std::mutex > lock( _counterLock );
			++ _counter;
			value = _counter;
		}
		return std::move( _draftsPath / std::to_string( value ) );
	}
		
private:
	boost::filesystem::path _rootPath;
	boost::filesystem::path _draftsPath;
	size_t _counter;
	std::mutex _counterLock;

	Drafts( const Drafts & rhs ) = delete;
	Drafts & operator= ( const Drafts & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_DRAFTS_H__
