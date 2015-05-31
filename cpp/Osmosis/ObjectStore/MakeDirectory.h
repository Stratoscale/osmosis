#ifndef __OSMOSIS_OBJECT_STORE_MAKE_DIRECTORY_H__
#define __OSMOSIS_OBJECT_STORE_MAKE_DIRECTORY_H__

#include <boost/filesystem.hpp>
#include <mutex>

namespace Osmosis {
namespace ObjectStore
{

class MakeDirectory
{
public:
	MakeDirectory( const boost::filesystem::path & path) :
		_path( path ),
		_directoryExists( false )
	{}

	void makeSureExists()
	{
		if ( _directoryExists )
			return;
		std::lock_guard< std::mutex > lock( _directoryExistsLock );
		if ( _directoryExists )
			return;
		makeDirectory();
	}

	void erase()
	{
		std::lock_guard< std::mutex > lock( _directoryExistsLock );
		bool removed = boost::filesystem::remove( _path );
		if ( not removed && _directoryExists )
			TRACE_ERROR( "File was removed under my feet " << _path );
		_directoryExists = false;
	}

private:
	boost::filesystem::path _path;
	std::mutex _directoryExistsLock;
	bool _directoryExists;

	void makeDirectory()
	{
		if ( boost::filesystem::is_directory( _path ) )
			return;
		const bool ret = boost::filesystem::create_directories( _path );
		ASSERT( ret );
		( void )ret;
		_directoryExists = true;
	}
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_MAKE_DIRECTORY_H__
