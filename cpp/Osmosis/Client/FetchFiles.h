#ifndef __OSMOSIS_CLIENT_FETCH_FILES_H__
#define __OSMOSIS_CLIENT_FETCH_FILES_H__

#include "Osmosis/ApplyFileStatus.h"
#include "Osmosis/Chain/Chain.h"
#include "Osmosis/Client/DigestDrafts.h"

namespace Osmosis {
namespace Client
{

class FetchFiles
{
public:
	FetchFiles( const boost::filesystem::path & directory, Chain::Chain & chain ) :
		_directory( directory ),
		_checkOut( chain.checkOut() ),
		_drafts( directory ),
		_fetchQueue( 1 ),
		_done( false ),
		_fetchRequested( 0 ),
		_fetchCompleted( 0 )
	{
		_threads.push_back( std::thread( & FetchFiles::fetchThreadEntryPoint, this ) );
		_threads.push_back( std::thread( & FetchFiles::afterVerifiedThreadEntryPoint, this ) );
	}

	~FetchFiles()
	{
		_fetchQueue.abort();
		for ( auto & i : _threads )
			if ( i.joinable() )
				i.join();
		try {
			_drafts.eraseDirectory();
		} CATCH_ALL_IGNORE( "Unable to erase drafts directory" );
	}

	void join()
	{
		for ( auto & i : _threads )
			i.join();
	}

	void noMoreFilesToFetch()
	{
		_done = true;
		possiblySignalProducerDone();
	}

	void fetch( const boost::filesystem::path & path, const FileStatus & status, const Hash & hash )
	{
		ASSERT( status.syncContent() );
		struct ToVerify task = { path, status, hash };
		_fetchRequested += 1;
		_fetchQueue.put( std::move( task ) );
	}

	boost::filesystem::path draftsPath() const
	{
		return _drafts.path();
	}

private:
	const boost::filesystem::path  _directory;
	Chain::CheckOut                _checkOut;
	ObjectStore::Drafts            _drafts;
	DigestDrafts                   _digestDrafts;
	ToVerifyTaskQueue              _fetchQueue;
	std::vector< std::thread >     _threads;
	bool                           _done;
	unsigned                       _fetchRequested;
	unsigned                       _fetchCompleted;

	void fetchThreadEntryPoint()
	{
		try {
			while ( true )
				fetchOne();
		} catch( ToVerifyTaskQueue::NoMoreTasksError & ) {
			_digestDrafts.toDigestTaskQueue().producerDone();
			TRACE_DEBUG( "Fetch thread done" );
		} CATCH_ALL_SUICIDE( "Fetch thread terminates" );
	}

	void fetchOne()
	{
		struct ToVerify entry = _fetchQueue.get();
		if ( entry.draft == "verify" )
			_checkOut.verify( entry.hash );
		entry.draft = _drafts.allocateFilename();
		_checkOut.getFile( entry.draft, entry.hash );
		_digestDrafts.toDigestTaskQueue().put( std::move( entry ) );
	}

	void afterVerifiedThreadEntryPoint()
	{
		try {
			while ( true )
				commitOne();
		} catch( ToVerifyTaskQueue::NoMoreTasksError & ) {
			TRACE_DEBUG( "After verified thread done" );
		} CATCH_ALL_SUICIDE( "After verified thread terminates" );
	}

	void commitOne()
	{
		auto task = _digestDrafts.digestedTaskQueue().get();
		if ( task.draft == boost::filesystem::path() ) {
			TRACE_INFO( "Reenqueueing '" << task.path << "' (" << task.hash <<
				"), it will be verified on each object store before another fetch attempt" );
			struct ToVerify reenqueued = { task.path, task.status, task.hash, "verify" };
			_fetchQueue.put( std::move( reenqueued ) );
			return;
		}
		boost::filesystem::path absolute = _directory / task.path;
		boost::filesystem::rename( task.draft, absolute );
		ApplyFileStatus( absolute, task.status ).applyExistingRegular();
		ASSERT_VERBOSE( FileStatus( absolute ) == task.status,
				absolute << ": " << FileStatus( absolute ) << " != " << task.status );
		_fetchCompleted += 1;
		possiblySignalProducerDone();
	}

	void possiblySignalProducerDone()
	{
		if ( _done and _fetchRequested <= _fetchCompleted ) {
			ASSERT( _fetchCompleted <= _fetchRequested );
			_fetchQueue.producerDone();
		}
	}

	FetchFiles( const FetchFiles & rhs ) = delete;
	FetchFiles & operator= ( const FetchFiles & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_FETCH_FILES_H__
