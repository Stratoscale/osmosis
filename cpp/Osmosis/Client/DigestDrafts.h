#ifndef __OSMOSIS_CLIENT_DIGEST_DRAFTS_H__
#define __OSMOSIS_CLIENT_DIGEST_DRAFTS_H__

namespace Osmosis {
namespace Client
{

class DigestDrafts
{
public:
	DigestDrafts() :
		_toDigestTaskQueue( 1 ),
		_digestedTaskQueue( digestionThreads() )
	{
		for ( unsigned i = 0; i < digestionThreads(); ++ i )
			_threads.push_back( std::thread( & DigestDrafts::threadsEntryPoint, this ) );
	}

	~DigestDrafts()
	{
		_toDigestTaskQueue.abort();
		_digestedTaskQueue.abort();
		for ( auto & i : _threads )
			if ( i.joinable() )
				i.join();
	}

	void join()
	{
		for ( auto & i : _threads )
			i.join();
	}

	ToVerifyTaskQueue & toDigestTaskQueue() { return _toDigestTaskQueue; }
	ToVerifyTaskQueue & digestedTaskQueue() { return _digestedTaskQueue; }

private:
	ToVerifyTaskQueue           _toDigestTaskQueue;
	ToVerifyTaskQueue           _digestedTaskQueue;
	std::vector< std::thread >  _threads; 

	void threadsEntryPoint()
	{
		try {
			try {
				while ( true )
					work();
			} catch ( ToVerifyTaskQueue::NoMoreTasksError ) {
				_digestedTaskQueue.producerDone();
				TRACE_DEBUG( "DigestThread done" );
			}
		} CATCH_ALL_SUICIDE( "Hash digestion thread terminated" );
	}

	void work()
	{
		BACKTRACE_BEGIN
		ToVerify task = _toDigestTaskQueue.get();
		if ( not CalculateHash::verify( task.draft, task.hash ) ) {
			TRACE_ERROR( "Draft failed hash verification: " << task.hash << " (" << task.path << ")" );
			boost::filesystem::remove( task.draft );
			task.draft = boost::filesystem::path();
		}
		_digestedTaskQueue.put( std::move( task ) );
		BACKTRACE_END
	}

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	DigestDrafts( const DigestDrafts & rhs ) = delete;
	DigestDrafts & operator= ( const DigestDrafts & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGEST_DRAFTS_H__
