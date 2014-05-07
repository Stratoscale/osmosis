#ifndef __OSMOSIS_CLIENT_FETCH_FILES_H__
#define __OSMOSIS_CLIENT_FETCH_FILES_H__

#include "Osmosis/ApplyFileStatus.h"

namespace Osmosis {
namespace Client
{

class FetchFiles
{
public:
	FetchFiles(     const boost::filesystem::path &  directory,
			const std::string &              hostname,
			unsigned short                   port ) :
		_directory( directory ),
		_getConnection( hostname, port ),
		_drafts( directory ),
		_fetchQueue( 1 )
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
		_fetchQueue.producerDone();
	}

	void fetch( const boost::filesystem::path & path, const FileStatus & status, const Hash & hash )
	{
		ASSERT( status.syncContent() );
		struct ToVerify task = { path, status, hash };
		_fetchQueue.put( std::move( task ) );
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::GET ) };
		_getConnection.socket().sendAllConcated( header, hash.raw() );
	}

private:
	const boost::filesystem::path  _directory;
	Connect                        _getConnection;
	ObjectStore::Drafts            _drafts;
	DigestDrafts                   _digestDrafts;
	ToVerifyTaskQueue              _fetchQueue;
	std::vector< std::thread >     _threads;

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
		entry.draft = _drafts.allocateFilename();
		Stream::SocketToFile transfer( _getConnection.socket(), entry.draft.string().c_str() );
		transfer.transfer();
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
		boost::filesystem::path absolute = _directory / task.path;
		boost::filesystem::rename( task.draft, absolute );
		ApplyFileStatus( absolute, task.status ).applyExistingRegular();
	}

	FetchFiles( const FetchFiles & rhs ) = delete;
	FetchFiles & operator= ( const FetchFiles & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_FETCH_FILES_H__
