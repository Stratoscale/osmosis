#ifndef __OSMOSIS_OBJECT_STORE_DIRECTORY_VALIDATOR_H__
#define __OSMOSIS_OBJECT_STORE_DIRECTORY_VALIDATOR_H__

#include <boost/filesystem.hpp>
#include <mutex>

namespace Osmosis {
namespace ObjectStore {

class DirectoryValidator
{
public:
	DirectoryValidator( const boost::filesystem::path & path) :
		_path( path ),
		_directoryExists( false )
	{ }

	void makeSureExists()
	{
                std::lock_guard< std::mutex > lock( _directoryExistsLock );
                if ( _directoryExists )
                        return;
                _directoryExists = true;
                if ( boost::filesystem::is_directory( _path ) )
                        return;
                const bool ret = boost::filesystem::create_directories( _path );
                ASSERT( ret );
		( void )ret;
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
};

} // namespace ObjectStore
} // namespace Osmosis

#endif // __OSMOSIS_OBJECT_STORE_DIRECTORY_VALIDATOR_H__
