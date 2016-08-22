#ifndef __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
#define __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__

#include <boost/filesystem.hpp>
#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/WaitCondition.h"

namespace Osmosis {
namespace Client
{

class CheckInProgress
{
public:
	CheckInProgress(	const boost::filesystem::path &  path,
				DigestDirectory &                digestDirectory,
				DigestedTaskQueue &              putQueue,
				unsigned                         reportIntervalSeconds );

	~CheckInProgress();

	void stop();

private:
        const boost::filesystem::path 	_path;
	DigestDirectory &		_digestDirectory;
	DigestedTaskQueue &		_putQueue;
	unsigned                       	_reportIntervalSeconds;
	std::thread                    	_thread;
	WaitCondition                   _threadStopCondition;

	void threadEntryPoint();

	void report( bool zeroIsDone );

	CheckInProgress( const CheckInProgress & rhs ) = delete;
	CheckInProgress & operator= ( const CheckInProgress & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
