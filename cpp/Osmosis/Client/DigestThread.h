#ifndef __OSMOSIS_CLIENT_DIGEST_THREAD_H__
#define __OSMOSIS_CLIENT_DIGEST_THREAD_H__

#include "Osmosis/Client/Typedefs.h"
#include "Osmosis/DirList.h"
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace Client
{

class DigestThread
{
public:
	static void task(       PathTaskQueue &                  inputQueue,
				DigestedTaskQueue &              outputQueue,
				bool                             md5,
				DirList &                        dirList,
				std::mutex &                     dirListMutex,
				const boost::filesystem::path &  directory )
	{
		try {
			DigestThread( inputQueue, outputQueue, md5, dirList, dirListMutex, directory ).go();
		} CATCH_ALL_SUICIDE( "Hash digestion thread terminated" );
	}

private:
	DigestThread(   PathTaskQueue &                  inputQueue,
			DigestedTaskQueue &              outputQueue,
			bool                             md5,
			DirList &                        dirList,
			std::mutex &                     dirListMutex,
			const boost::filesystem::path &  directory ) :
		_inputQueue( inputQueue ),
		_outputQueue( outputQueue ),
		_md5( md5 ),
		_dirList( dirList ),
		_dirListMutex( dirListMutex ),
		_directory( directory )
	{}

	void work()
	{
		BACKTRACE_BEGIN
		boost::filesystem::path task = _inputQueue.get();
		boost::filesystem::path absolute = _directory / task;
		Hash hash = _md5 ? CalculateHash::MD5( absolute ) : CalculateHash::SHA1( absolute );
		Digested result = { task, hash };
		_outputQueue.put( std::move( result ) );
		std::lock_guard< std::mutex > lock( _dirListMutex );
		_dirList.setHash( task, hash );
		BACKTRACE_END
	}

	void go()
	{
		try {
			while ( true )
				work();
		} catch ( PathTaskQueue::NoMoreTasksError ) {
			_outputQueue.producerDone();
			TRACE_DEBUG( "DigestThread done" );
		}
	}

	PathTaskQueue &                _inputQueue;
	DigestedTaskQueue &            _outputQueue;
	const bool                     _md5;
	DirList &                      _dirList;
	std::mutex &                   _dirListMutex;
	const boost::filesystem::path  _directory; 

	DigestThread( const DigestThread & rhs ) = delete;
	DigestThread & operator= ( const DigestThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGEST_THREAD_H__
