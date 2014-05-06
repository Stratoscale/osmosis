#ifndef __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__
#define __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__

#include "Osmosis/Client/DigestThread.h"
#include "Common/NumberOfCPUs.h"

namespace Osmosis {
namespace Client
{

class DigestDirectory
{
public:
	DigestDirectory(        const boost::filesystem::path &  directory,
				bool                             md5 ) :
		_directory( directory ),
		_toDigestTaskQueue( 1 ),
		_digestedQueue( digestionThreads() )
	{
		for ( unsigned i = 0; i < digestionThreads(); ++ i )
			_threads.push_back( std::thread(
				DigestThread::task,
				std::ref( _toDigestTaskQueue ),
				std::ref( _digestedQueue ),
				md5,
				std::ref( _dirList ),
				std::ref( _dirListMutex ) ) );
		_threads.push_back( std::thread( & DigestDirectory::threadEntryPoint, this ) );
	}

	~DigestDirectory()
	{
		_toDigestTaskQueue.abort();
		_digestedQueue.abort();
		for ( auto & i : _threads )
			if ( i.joinable() )
				i.join();
	}

	void join()
	{
		for ( auto & i : _threads )
			i.join();
	}

	DigestedTaskQueue & digestedQueue() { return _digestedQueue; }
	DirList & dirList() { return _dirList; }

private:
	const boost::filesystem::path  _directory;
	DirList                        _dirList;
	std::mutex                     _dirListMutex;
	PathTaskQueue                  _toDigestTaskQueue;
	DigestedTaskQueue              _digestedQueue; 
	std::vector< std::thread >     _threads; 

	void threadEntryPoint()
	{
		try {
			traverseDirectoryAndFillUpDigestionQueue();
		} CATCH_ALL_SUICIDE( "Digest directory walking thread terminated" )
	}

	void traverseDirectoryAndFillUpDigestionQueue()
	{
		std::string prefix = ( _directory / "" ).string();
		for ( boost::filesystem::recursive_directory_iterator i( _directory );
				i != boost::filesystem::recursive_directory_iterator();
				++ i ) {
			boost::filesystem::path path = i->path();
			{
				std::lock_guard< std::mutex > lock( _dirListMutex );
				_dirList.add( path );
			}
//TODO: only if not device or symlink
			_toDigestTaskQueue.put( std::move( path ) );
		}
		_toDigestTaskQueue.producerDone();
	}

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	DigestDirectory( const DigestDirectory & rhs ) = delete;
	DigestDirectory & operator= ( const DigestDirectory & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__
