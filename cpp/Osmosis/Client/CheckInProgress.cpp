#include <fstream>
#include <boost/filesystem.hpp>
#include "Osmosis/Client/CheckInProgress.h"
#include "Osmosis/Client/DigestDirectory.h"
#include "Common/ProgressPercent.h"

namespace Osmosis {
namespace Client
{

CheckInProgress::CheckInProgress(	const boost::filesystem::path &  path,
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

CheckInProgress::~CheckInProgress()
{
	stop();
}

void CheckInProgress::stop()
{
	if ( ! _thread.joinable() )
		return;
	_threadStopCondition.stop();
	_thread.join();
	report( true );
}

void CheckInProgress::threadEntryPoint()
{
	while ( _threadStopCondition.sleep() )
		report( false );
}

void CheckInProgress::report( bool zeroIsDone )
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

} // namespace Client
} // namespace Osmosis
