#include <fcntl.h>
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/ApplyFileStatus.h"
#include "Osmosis/Chain/Chain.h"
#include "Osmosis/Client/DigestDrafts.h"

namespace Osmosis {
namespace Client
{

FetchFiles::FetchFiles( const boost::filesystem::path & directory, Chain::Chain & chain ) :
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

FetchFiles::~FetchFiles()
{
	_fetchQueue.abort();
	for ( auto & i : _threads )
		if ( i.joinable() )
			i.join();
	try {
		_drafts.eraseDirectory();
	} CATCH_ALL_IGNORE( "Unable to erase drafts directory" );
}

void FetchFiles::join()
{
	for ( auto & i : _threads )
		i.join();
}

void FetchFiles::noMoreFilesToFetch()
{
	_done = true;
	possiblySignalProducerDone();
}

void FetchFiles::fetch( const boost::filesystem::path & path, const FileStatus & status, const Hash & hash )
{
	ASSERT( status.syncContent() );
	struct ToVerify task = { path, status, hash };
	_fetchRequested += 1;
	_fetchQueue.put( std::move( task ) );
}

boost::filesystem::path FetchFiles::draftsPath() const
{
	return _drafts.path();
}

FetchFiles::Stats FetchFiles::stats()
{
	return { _fetchRequested, _fetchCompleted, static_cast< unsigned >( _digestDrafts.toDigestTaskQueue().size() ) };
}
const Chain::CheckOut::GetCountStats & FetchFiles::checkOutGetCount() { return _checkOut.getCount(); }

void FetchFiles::fetchThreadEntryPoint()
{
	try {
		while ( true )
			fetchOne();
	} catch( ToVerifyTaskQueue::NoMoreTasksError & ) {
		_digestDrafts.toDigestTaskQueue().producerDone();
		TRACE_DEBUG( "Fetch thread done" );
	} CATCH_ALL_SUICIDE( "Fetch thread terminates" );
}

void FetchFiles::fetchOne()
{
	BACKTRACE_BEGIN
	struct ToVerify entry = _fetchQueue.get();
	if ( entry.draft == "verify" )
		_checkOut.verify( entry.hash );
	entry.draft = _drafts.allocateFilename();
	_checkOut.getFile( entry.draft, entry.hash );
	_digestDrafts.toDigestTaskQueue().put( std::move( entry ) );
	BACKTRACE_END
}

void FetchFiles::afterVerifiedThreadEntryPoint()
{
	try {
		while ( true )
			commitOne();
	} catch( ToVerifyTaskQueue::NoMoreTasksError & ) {
		TRACE_DEBUG( "After verified thread done" );
	} CATCH_ALL_SUICIDE( "After verified thread terminates" );
}

void FetchFiles::commitOne()
{
	BACKTRACE_BEGIN
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
	BACKTRACE_END
}

void FetchFiles::possiblySignalProducerDone()
{
	if ( _done and _fetchRequested <= _fetchCompleted ) {
		ASSERT( _fetchCompleted <= _fetchRequested );
		_fetchQueue.producerDone();
	}
}

} // namespace Client
} // namespace Osmosis
