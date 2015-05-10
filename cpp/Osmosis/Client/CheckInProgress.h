#ifndef __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
#define __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Stream/BufferToSocket.h"
#include "Common/NumberOfCPUs.h"
#include "Common/ProgressPercent.h"
#include "Common/ThreadStopCondition.h"

namespace Osmosis {
namespace Client
{

class CheckInProgress
{
public:
	CheckInProgress(	const boost::filesystem::path &  path,
				DigestDirectory &                digestDirectory,
				DigestedTaskQueue &              putQueue,
				unsigned                         reportIntervalSeconds ):
		_path( path ),
		_digestDirectory( digestDirectory ),
		_putQueue( putQueue ),
		_threadStopCondition( reportIntervalSeconds )
	{
		_thread = std::thread( & CheckInProgress::threadEntryPoint, this );
	}

	~CheckInProgress()
	{
		stop();
	}

	void stop()
	{
		if ( ! _thread.joinable() )
			return;
		_threadStopCondition.stop();
		_thread.join();
		report( true );
	}

private:
        const boost::filesystem::path 	_path;
	DigestDirectory &		_digestDirectory;
	DigestedTaskQueue &		_putQueue;
	unsigned                       	_reportIntervalSeconds;
	std::thread                    	_thread;
	ThreadStopCondition             _threadStopCondition;

	void threadEntryPoint()
	{
		while ( _threadStopCondition.sleep() )
			report( false );
	}

	void report( bool zeroIsDone )
	{
		unsigned digestionPercent = ProgressPercent::calc(
			_digestDirectory.toDigestTaskQueue().getCount(), _digestDirectory.toDigestTaskQueue().putCount(), zeroIsDone );
		unsigned checkExistingPercent = ProgressPercent::calc(
			_digestDirectory.digestedQueue().getCount(), _digestDirectory.toDigestTaskQueue().putCount(), zeroIsDone );
		unsigned putPercent = ProgressPercent::calc(
			_putQueue.getCount(), _putQueue.putCount(), zeroIsDone );
		unsigned percent = std::min( { digestionPercent, checkExistingPercent, putPercent } );
		TRACE_INFO( "Status: checkin " << percent << "%. digestion: " <<
			_digestDirectory.toDigestTaskQueue().getCount() << "/" << _digestDirectory.toDigestTaskQueue().putCount() << " " << digestionPercent
			<< "% check: " <<
			_digestDirectory.digestedQueue().getCount() << "/" << _digestDirectory.toDigestTaskQueue().putCount() << " " << checkExistingPercent
			<< "% put: " <<
			_putQueue.getCount() << "/" << _putQueue.putCount() << " " << putPercent << "%" );
		if ( _path != boost::filesystem::path( "" ) ) {
			std::ofstream output( _path.string() );
			output <<
				"{ \"state\": \"checkin\", \"percent\": " << percent << ", \"digestion\": { " <<
				"\"done\": " << _digestDirectory.toDigestTaskQueue().getCount() <<
				", \"total\": " << _digestDirectory.toDigestTaskQueue().putCount() << "}, \"checkExisting\": { " <<
				"\"done\": " << _digestDirectory.digestedQueue().getCount() <<
				", \"total\": " << _digestDirectory.toDigestTaskQueue().putCount() << "}, \"put\": { " <<
				"\"done\": " << _putQueue.getCount() <<
				", \"total\": " << _putQueue.putCount() << "}}";
		}
	}

	CheckInProgress( const CheckInProgress & rhs ) = delete;
	CheckInProgress & operator= ( const CheckInProgress & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
