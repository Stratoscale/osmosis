#ifndef __OSMOSIS_OBJECT_STORE_DRAFTS_H__
#define __OSMOSIS_OBJECT_STORE_DRAFTS_H__

#include <boost/algorithm/string/predicate.hpp>
#include <mutex>

namespace Osmosis {
namespace ObjectStore
{

class Drafts
{
public:
	Drafts( const boost::filesystem::path & rootPath ) :
		_draftsPath( rootPath / DirectoryNames::DRAFTS ),
		_counter( 0 ),
		_pidPrefix(pidPrefix())
	{
		if ( not boost::filesystem::is_directory( _draftsPath ) )
			boost::filesystem::create_directories( _draftsPath );
		cleanUp();
	}

	void cleanUp()
	{
		std::time_t now = std::time(nullptr);
		for ( auto i = boost::filesystem::directory_iterator( _draftsPath );
				i != boost::filesystem::directory_iterator(); ++ i ) {
			if ( not boost::starts_with( i->path().filename().string(), _pidPrefix ) ) {
				if ( boost::filesystem::last_write_time( i->path() ) + SAFE_TO_ERASE_FROM_ANOTHER_PID > now ) {
					TRACE_WARNING( "Will not erase a leftover draft from another PID " << i->path() );
					continue;
				}
				TRACE_WARNING( "Erasing a leftover draft from another PID " << i->path() );

			}
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
		return std::move( _draftsPath / ( _pidPrefix + std::to_string( value ) ) );
	}

	boost::filesystem::path path() const
	{
		return _draftsPath;
	}
		
private:
	enum { SAFE_TO_ERASE_FROM_ANOTHER_PID = 60 * 60 };

	boost::filesystem::path  _draftsPath;
	size_t                   _counter;
	std::mutex               _counterLock;
	std::string              _pidPrefix;

	static std::string pidPrefix()
	{
		return std::to_string(getpid()) + ".";
	}

	Drafts( const Drafts & rhs ) = delete;
	Drafts & operator= ( const Drafts & rhs ) = delete;
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_DRAFTS_H__
