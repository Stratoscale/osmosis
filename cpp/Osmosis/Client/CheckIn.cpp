#include "Osmosis/Client/CheckIn.h"
#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Client/CheckInProgress.h"

namespace Osmosis {
namespace Client
{

CheckIn::CheckIn(        const boost::filesystem::path &  directory,
		const std::string &              label,
		Chain::ObjectStoreInterface &    objectStore,
		bool                             md5,
		const boost::filesystem::path &  progressReport,
		unsigned                         progressReportIntervalSeconds,
		bool                             followSymlinks ) :
	_label( label ),
	_md5( md5 ),
	_digestDirectory( directory, md5, _ignores, followSymlinks ),
	_putConnection( objectStore.connect() ),
	_putQueue( CHECK_EXISTING_THREADS ),
	_checkInProgress( progressReport, _digestDirectory, _putQueue, progressReportIntervalSeconds )
{
	BACKTRACE_BEGIN
	for ( unsigned i = 0; i < CHECK_EXISTING_THREADS; ++ i )
		_threads.push_back( std::thread(
			CheckExistingThread::task, std::ref( _digestDirectory.digestedQueue() ), std::ref( _putQueue ), std::ref( objectStore ),
			std::ref( _checkExistingAlreadyProcessed ), std::ref( _checkExistingAlreadyProcessedLock ) ) );
	_threads.push_back( std::thread( PutThread::task, std::ref( _putQueue ), std::ref( * _putConnection ), std::ref( directory ) ) );
	BACKTRACE_END
}

CheckIn::~CheckIn()
{
	_putQueue.abort();
	for ( auto & i : _threads )
		if ( i.joinable() )
			i.join();
}

void CheckIn::go()
{
	BACKTRACE_BEGIN
	_digestDirectory.join();
	for ( auto & i : _threads )
		i.join();
	Hash hash = putDirList();
	_putConnection->setLabel( hash, _label );
	BACKTRACE_END
}

Hash CheckIn::putDirList()
{
	BACKTRACE_BEGIN
	std::ostringstream out;
	out << _digestDirectory.dirList();
	std::string text( out.str() );
	Hash hash = CalculateHash::SHA1( text.c_str(), text.size() );
	if ( _putConnection->exists( hash ) )
		return hash;
	_putConnection->putString( text, hash );
	return hash;
	BACKTRACE_END
}

} // namespace Client
} // namespace Osmosis
