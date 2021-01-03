#ifndef __OSMOSIS_CLIENT_PUT_THREAD_H__
#define __OSMOSIS_CLIENT_PUT_THREAD_H__

namespace Osmosis {
namespace Client
{

class PutThread
{
public:
	static void task(       DigestedTaskQueue &                      inputQueue,
				Chain::ObjectStoreConnectionInterface &  connection,
				const boost::filesystem::path &          directory )
	{
		try {
			PutThread( inputQueue, connection, directory ).go();
		} CATCH_ALL_SUICIDE( "Put thread terminated" )
	}

private:
	PutThread(      DigestedTaskQueue &                      inputQueue,
			Chain::ObjectStoreConnectionInterface &  connection,
			const boost::filesystem::path &          directory ):
		_inputQueue( inputQueue ),
		_connection( connection ),
		_directory( directory )
	{}

	void work()
	{
		BACKTRACE_BEGIN
		Digested task = _inputQueue.get();
		boost::filesystem::path absolute = _directory / task.path;
		_connection.putFile( absolute, task.hash );
		TRACE_DEBUG( "Transferred file " << task.path );
		BACKTRACE_END
	}

	void go()
	{
		try {
			while ( true )
				work();
		} catch ( DigestedTaskQueue::NoMoreTasksError& ) {
			TRACE_DEBUG( "PutThread done" );
		}
	}

	DigestedTaskQueue &                      _inputQueue;
	Chain::ObjectStoreConnectionInterface &  _connection;
	const boost::filesystem::path            _directory;

	PutThread( const PutThread & rhs ) = delete;
	PutThread & operator= ( const PutThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_PUT_THREAD_H__
