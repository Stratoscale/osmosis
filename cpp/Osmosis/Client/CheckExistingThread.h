#ifndef __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
#define __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__

#include "Osmosis/Chain/ObjectStoreInterface.h"

namespace Osmosis {
namespace Client
{

class CheckExistingThread
{
public:
	typedef std::set< Hash > AlreadyProcessed;

	static void task(       DigestedTaskQueue &            inputQueue,
				DigestedTaskQueue &            outputQueue,
				Chain::ObjectStoreInterface &  objectStore,
				AlreadyProcessed &             alreadyProccessed,
				std::mutex &                   alreadyProccessedLock )

	{
		try {
			CheckExistingThread( inputQueue, outputQueue, objectStore,
					alreadyProccessed, alreadyProccessedLock ).go();
		} CATCH_ALL_SUICIDE( "Exists checking thread terminated" );
	}

private:
	CheckExistingThread(    DigestedTaskQueue &            inputQueue,
				DigestedTaskQueue &            outputQueue,
				Chain::ObjectStoreInterface &  objectStore,
				AlreadyProcessed &             alreadyProccessed,
				std::mutex &                   alreadyProccessedLock ):
		_inputQueue( inputQueue ),
		_outputQueue( outputQueue ),
		_alreadyProccessed( alreadyProccessed ),
		_alreadyProccessedLock( alreadyProccessedLock ),
		_connection( objectStore.connect() )
	{}

	void work()
	{
		BACKTRACE_BEGIN
		Digested task = _inputQueue.get();
		if ( alreadyProccessed( task.hash ) )
			return;
		if ( _connection->exists( task.hash ) )
			TRACE_DEBUG( " EXISTS " << task.hash );
		else {
			TRACE_DEBUG( "!EXISTS " << task.hash );
			_outputQueue.put( std::move( task ) );
		}
		BACKTRACE_END
	}

	void go()
	{
		BACKTRACE_BEGIN
		try {
			while ( true )
				work();
		} catch ( DigestedTaskQueue::NoMoreTasksError ) {
			_outputQueue.producerDone();
			TRACE_DEBUG( "CheckExistingThread done" );
		}
		BACKTRACE_END
	}

	bool alreadyProccessed( const Hash & hash )
	{
		std::lock_guard< std::mutex > lock( _alreadyProccessedLock );
		if ( _alreadyProccessed.find( hash ) != _alreadyProccessed.end() )
			return true;
		_alreadyProccessed.emplace( hash );
		return false;
	}

	DigestedTaskQueue &                                       _inputQueue;
	DigestedTaskQueue &                                       _outputQueue;
	AlreadyProcessed &                                        _alreadyProccessed;
	std::mutex &                                              _alreadyProccessedLock;
	std::unique_ptr< Chain::ObjectStoreConnectionInterface >  _connection;

	CheckExistingThread( const CheckExistingThread & rhs ) = delete;
	CheckExistingThread & operator= ( const CheckExistingThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
