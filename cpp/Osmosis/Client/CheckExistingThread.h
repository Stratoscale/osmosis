#ifndef __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
#define __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__

#include "Osmosis/Client/Connect.h"

namespace Osmosis {
namespace Client
{

class CheckExistingThread
{
public:
	static void task(       DigestedTaskQueue &  inputQueue,
				DigestedTaskQueue &  outputQueue,
				const std::string    hostname,
				unsigned short       port )

	{
		try {
			CheckExistingThread( inputQueue, outputQueue, hostname, port ).go();
		} CATCH_ALL_SUICIDE( "Exists checking thread terminated" );
	}

private:
	CheckExistingThread(    DigestedTaskQueue &  inputQueue,
				DigestedTaskQueue &  outputQueue,
				const std::string &  hostname,
				unsigned short       port ):
		_inputQueue( inputQueue ),
		_outputQueue( outputQueue ),
		_connect( hostname, port )
	{}

	void work()
	{
		Digested task = _inputQueue.get();
		struct Tongue::Header header = { static_cast< unsigned char >( Tongue::Opcode::IS_EXISTS ) };
		_connect.socket().sendAllConcated( header, task.hash.raw() );
		auto response = _connect.socket().recieveAll< struct Tongue::IsExistsResponse >();
		if ( response.response == static_cast< unsigned char >( Tongue::IsExists::NO ) ) {
			TRACE_DEBUG( "!EXISTS " << task.hash );
			_outputQueue.put( std::move( task ) );
		} else
			TRACE_DEBUG( " EXISTS " << task.hash );
	}

	void go()
	{
		try {
			while ( true )
				work();
		} catch ( DigestedTaskQueue::NoMoreTasksError ) {
			_outputQueue.producerDone();
			TRACE_DEBUG( "CheckExistingThread done" );
		}
	}

	DigestedTaskQueue &  _inputQueue;
	DigestedTaskQueue &  _outputQueue;
	Connect              _connect; 

	CheckExistingThread( const CheckExistingThread & rhs ) = delete;
	CheckExistingThread & operator= ( const CheckExistingThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
