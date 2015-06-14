#ifndef __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__
#define __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__

#include "Osmosis/Client/DigestThread.h"
#include "Osmosis/Client/Ignores.h"
#include "Common/NumberOfCPUs.h"

namespace Osmosis {
namespace Client
{

class DigestDirectory
{
public:
	DigestDirectory(        const boost::filesystem::path &  directory,
				bool                             md5,
				const Ignores &                  ignores ) :
		_directory( directory ),
		_ignores( ignores ),
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
				std::ref( _dirListMutex ),
				directory ) );
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
	const PathTaskQueue & toDigestTaskQueue() const { return _toDigestTaskQueue; }
	DirList & dirList() { return _dirList; }

private:
	const boost::filesystem::path  _directory;
	const Ignores &                _ignores;
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
		BACKTRACE_BEGIN
		static const unsigned DELIMITER_SIZE = 1;
		unsigned prefixLength = _directory.string().size() + DELIMITER_SIZE;
		for ( boost::filesystem::recursive_directory_iterator i( _directory );
				i != boost::filesystem::recursive_directory_iterator();
				++ i ) {
			boost::filesystem::path path = i->path();
			if ( _ignores.ignored( path ) ) {
				i.no_push( true );
				continue;
			}
			FileStatus status( path );
			if ( not status.isSymlink() and status.isSocket() ) {
				TRACE_WARNING( "Will not digest socket file: " << path );
				continue;
			}
			boost::filesystem::path relative = path.string().substr( prefixLength );
			{
				std::lock_guard< std::mutex > lock( _dirListMutex );
				_dirList.add( relative, status );
			}
			if ( status.syncContent() )
				_toDigestTaskQueue.put( std::move( relative ) );
		}
		_toDigestTaskQueue.producerDone();
		BACKTRACE_END
	}

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	DigestDirectory( const DigestDirectory & rhs ) = delete;
	DigestDirectory & operator= ( const DigestDirectory & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGEST_DIRECTORY_H__
