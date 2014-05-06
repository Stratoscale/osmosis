#ifndef __OSMOSIS_CLIENT_DIGEST_THREAD_H__
#define __OSMOSIS_CLIENT_DIGEST_THREAD_H__

#include "Osmosis/Client/Typedefs.h"
#include "Osmosis/Client/DirList.h"
#include "Osmosis/Debug.h"

namespace Osmosis {
namespace Client
{

class DigestThread
{
public:
	static void task(       PathTaskQueue &      inputQueue,
				DigestedTaskQueue &  outputQueue,
				bool                 md5,
				DirList &            dirList,
				std::mutex &         dirListMutex )
	{
		try {
			DigestThread( inputQueue, outputQueue, md5, dirList, dirListMutex ).go();
		} CATCH_ALL_SUICIDE( "Hash digestion thread terminated" );
	}

private:
	DigestThread(   PathTaskQueue &      inputQueue,
			DigestedTaskQueue &  outputQueue,
			bool                 md5,
			DirList &            dirList,
			std::mutex &         dirListMutex ) :
		_inputQueue( inputQueue ),
		_outputQueue( outputQueue ),
		_md5( md5 ),
		_dirList( dirList ),
		_dirListMutex( dirListMutex )
	{}

	void work()
	{
		boost::filesystem::path task = _inputQueue.get();
		Hash hash = _md5 ? CalculateHash::MD5( task ) : CalculateHash::SHA1( task );
		Digested result = { task, hash };
		_outputQueue.put( std::move( result ) );
		std::lock_guard< std::mutex > lock( _dirListMutex );
		_dirList.setHash( task, hash );
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

	PathTaskQueue &      _inputQueue;
	DigestedTaskQueue &  _outputQueue;
	bool                 _md5;
	DirList &            _dirList;
	std::mutex &         _dirListMutex; 

	DigestThread( const DigestThread & rhs ) = delete;
	DigestThread & operator= ( const DigestThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGEST_THREAD_H__
