#ifndef __OSMOSIS_CLIENT_CHECK_IN_H__
#define __OSMOSIS_CLIENT_CHECK_IN_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/CheckInProgress.h"
#include "Common/NumberOfCPUs.h"

namespace Osmosis {
namespace Client
{

class CheckIn
{
public:
	CheckIn(        const boost::filesystem::path &  directory,
			const std::string &              label,
			Chain::ObjectStoreInterface &    objectStore,
			bool                             md5,
			const boost::filesystem::path &  progressReport,
			unsigned                         progressReportIntervalSeconds,
			bool                             followSymlinks );

	~CheckIn();

	void go();

private:
	enum {
		CHECK_EXISTING_THREADS = 10,
	};

	const std::string                                         _label;
	const bool                                                _md5;
	CheckExistingThread::AlreadyProcessed                     _checkExistingAlreadyProcessed;
	std::mutex                                                _checkExistingAlreadyProcessedLock;
	Ignores                                                   _ignores;
	DigestDirectory                                           _digestDirectory;
	std::unique_ptr< Chain::ObjectStoreConnectionInterface >  _putConnection;
	DigestedTaskQueue                                         _putQueue;
	std::vector< std::thread >                                _threads;
	CheckInProgress						  _checkInProgress;

	Hash putDirList();

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	CheckIn( const CheckIn & rhs ) = delete;
	CheckIn & operator= ( const CheckIn & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_H__
