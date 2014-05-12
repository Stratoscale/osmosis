#ifndef __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
#define __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__

#include "Osmosis/Client/Connect.h"

namespace Osmosis {
namespace Client
{

class CheckExistingThread
{
public:
	typedef std::set< Hash > AlreadyProcessed;

	static void task(       DigestedTaskQueue &  inputQueue,
				DigestedTaskQueue &  outputQueue,
				const std::string    hostname,
				unsigned short       port,
				AlreadyProcessed &   alreadyProccessed,
				std::mutex &         alreadyProccessedLock )

	{
		try {
			CheckExistingThread( inputQueue, outputQueue, hostname, port,
					alreadyProccessed, alreadyProccessedLock ).go();
		} CATCH_ALL_SUICIDE( "Exists checking thread terminated" );
	}

private:
	CheckExistingThread(    DigestedTaskQueue &  inputQueue,
				DigestedTaskQueue &  outputQueue,
				const std::string &  hostname,
				unsigned short       port,
				AlreadyProcessed &   alreadyProccessed,
				std::mutex &         alreadyProccessedLock ):
		_inputQueue( inputQueue ),
		_outputQueue( outputQueue ),
		_connect( hostname, port ),
		_alreadyProccessed( alreadyProccessed ),
		_alreadyProccessedLock( alreadyProccessedLock )
	{}

	void work()
	{
		Digested task = _inputQueue.get();
		if ( alreadyProccessed( task.hash ) )
			return;
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

	bool alreadyProccessed( const Hash & hash )
	{
		std::lock_guard< std::mutex > lock( _alreadyProccessedLock );
		if ( _alreadyProccessed.find( hash ) != _alreadyProccessed.end() )
			return true;
		_alreadyProccessed.emplace( hash );
		return false;
	}

	DigestedTaskQueue &  _inputQueue;
	DigestedTaskQueue &  _outputQueue;
	Connect              _connect; 
	AlreadyProcessed &   _alreadyProccessed;
	std::mutex &         _alreadyProccessedLock;

	CheckExistingThread( const CheckExistingThread & rhs ) = delete;
	CheckExistingThread & operator= ( const CheckExistingThread & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_EXISTING_THREAD_H__
