#ifndef __OSMOSIS_CLIENT_TRANSFER_THREAD_H__
#define __OSMOSIS_CLIENT_TRANSFER_THREAD_H__

namespace Osmosis {
namespace Client
{

class TransferThread
{
public:
	static void task(       DigestedTaskQueue &            inputQueue,
				Chain::Chain &                 chain,
				Chain::ObjectStoreInterface &  destination )
	{
		try {
			TransferThread( inputQueue, chain, destination ).go();
		} CATCH_ALL_SUICIDE( "Transfer thread terminated" );
	}

private:
	TransferThread( DigestedTaskQueue &            inputQueue,
			Chain::Chain &                 chain,
			Chain::ObjectStoreInterface &  destination ) :
		_inputQueue( inputQueue ),
		_checkOut( chain.checkOut() ),
		_destinationConnection( std::move( destination.connect() ) )
	{}

	void work()
	{
		BACKTRACE_BEGIN
		struct Digested task = _inputQueue.get();
		try {
			std::string content = _checkOut.getString( task.hash );
			_destinationConnection->putString( content, task.hash );
		} catch (...) {
			TRACE_ERROR( "While transferring hash: " << task.hash );
			throw;
		}
		BACKTRACE_END
	}

	void go()
	{
		try {
			while ( true )
				work();
		} catch ( DigestedTaskQueue::NoMoreTasksError ) {
			TRACE_DEBUG( "Transfer thread done" );
		}
	}

	DigestedTaskQueue &                                       _inputQueue;
	Chain::CheckOut                                           _checkOut;
	std::unique_ptr< Chain::ObjectStoreConnectionInterface >  _destinationConnection;

	TransferThread( const TransferThread & rhs ) = delete;
	TransferThread & operator= ( const TransferThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_TRANSFER_THREAD_H__
