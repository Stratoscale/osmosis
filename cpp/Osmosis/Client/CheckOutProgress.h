#ifndef __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Stream/BufferToSocket.h"

namespace Osmosis {
namespace Client
{

class CheckOutProgress
{
public:
	CheckOutProgress(       const boost::filesystem::path &  path,
				DigestDirectory &                digestDirectory,
				unsigned                         reportIntervalSeconds ):
		_path( path ),
		_digestDirectory( digestDirectory ),
		_reportIntervalSeconds( reportIntervalSeconds ),
		_fetchFiles( NULL )
	{
		_thread = std::thread( & CheckOutProgress::threadEntryPoint, this );
	}

	~CheckOutProgress()
	{
		stop();
	}

	void stop()
	{
		if ( ! _thread.joinable() )
			return;
		_stop.notify_one();
		_thread.join();
		report( true );
	}

	void setFetchFiles( FetchFiles & fetchFiles )
	{
		ASSERT( _fetchFiles == NULL );
		_fetchFiles = & fetchFiles;
	}

private:
        const boost::filesystem::path  _path;
	DigestDirectory &              _digestDirectory;
	unsigned                       _reportIntervalSeconds;
	FetchFiles *                   _fetchFiles;
	std::thread                    _thread;
	std::mutex                     _stopLock;
	std::condition_variable        _stop;

	bool sleep()
	{
		std::unique_lock< std::mutex > lock( _stopLock );
		auto result = _stop.wait_for( lock, std::chrono::seconds( _reportIntervalSeconds ) );
		return result == std::cv_status::timeout;
	}

	void threadEntryPoint()
	{
		while ( sleep() )
			report( false );
	}

	void report( bool zeroIsDone )
	{
		if ( _fetchFiles != NULL ) {
			FetchFiles::Stats stats = _fetchFiles->stats();
			const Chain::CheckOut::GetCountStats & checkOutGetCount = _fetchFiles->checkOutGetCount();
			TRACE_INFO( "Status: fetching. fetchesRequested: " << stats.fetchesRequested <<
					", completed: " << stats.fetchesCompleted <<
					", digestQueueLength: " << stats.digestQueueLength );
			std::ostringstream chainCount;
			for ( auto count : checkOutGetCount )
				chainCount << count << ' ';
			TRACE_INFO( "Get count per chain component: " << chainCount.str() );
			if ( _path != boost::filesystem::path( "" ) ) {
				std::ofstream output( _path.string() );
				unsigned percent = ProgressPercent::calc(
					stats.fetchesCompleted, stats.fetchesRequested, zeroIsDone );
				output << "{ \"state\": \"fetching\", \"percent\": " << percent <<
					", \"fetchesRequested\": " << stats.fetchesRequested <<
					", \"fetchesCompleted\": " << stats.fetchesCompleted <<
					", \"digestQueueLength\": " << stats.digestQueueLength <<
					", \"chainGetCount\": [";
				bool first = true;
				for ( auto count : checkOutGetCount ) {
					if ( first )
						first = false;
					else
						output << ", ";
					output << count;
				}
				output << "]}";
			}
		} else {
			unsigned digestionPercent = ProgressPercent::calc(
				_digestDirectory.toDigestTaskQueue().getCount(), _digestDirectory.toDigestTaskQueue().putCount(), zeroIsDone );
			TRACE_INFO( "Status: digesting " << digestionPercent << "% " <<
				_digestDirectory.toDigestTaskQueue().getCount() << "/" << _digestDirectory.toDigestTaskQueue().putCount() );
			if ( _path != boost::filesystem::path( "" ) ) {
				std::ofstream output( _path.string() );
				output <<
					"{ \"state\": \"digesting\", \"percent\": " << digestionPercent << ", \"digestion\": { " <<
					"\"done\": " << _digestDirectory.toDigestTaskQueue().getCount() <<
					", \"total\": " << _digestDirectory.toDigestTaskQueue().putCount() << "}}";
			}
		}
	}

	CheckOutProgress( const CheckOutProgress & rhs ) = delete;
	CheckOutProgress & operator= ( const CheckOutProgress & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__
