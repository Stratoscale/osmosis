#ifndef __OSMOSIS_CLIENT_CHECK_IN_H__
#define __OSMOSIS_CLIENT_CHECK_IN_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Stream/BufferToSocket.h"
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
			bool                             md5 ) :
		_label( label ),
		_md5( md5 ),
		_digestDirectory( directory, md5 ),
		_putConnection( objectStore.connect() ),
		_putQueue( CHECK_EXISTING_THREADS )
	{
		for ( unsigned i = 0; i < CHECK_EXISTING_THREADS; ++ i )
			_threads.push_back( std::thread(
				CheckExistingThread::task, std::ref( _digestDirectory.digestedQueue() ), std::ref( _putQueue ), std::ref( objectStore ),
				std::ref( _checkExistingAlreadyProcessed ), std::ref( _checkExistingAlreadyProcessedLock ) ) );
		_threads.push_back( std::thread( PutThread::task, std::ref( _putQueue ), std::ref( * _putConnection ), std::ref( directory ) ) );
	}

	~CheckIn()
	{
		_putQueue.abort();
		for ( auto & i : _threads )
			if ( i.joinable() )
				i.join();
	}

	void go()
	{
		_digestDirectory.join();
		for ( auto & i : _threads )
			i.join();
		Hash hash = putDirList();
		_putConnection->setLabel( hash, _label );
	}

private:
	enum {
		CHECK_EXISTING_THREADS = 10,
	};

	const std::string                                         _label;
	const bool                                                _md5;
	CheckExistingThread::AlreadyProcessed                     _checkExistingAlreadyProcessed;
	std::mutex                                                _checkExistingAlreadyProcessedLock;
	DigestDirectory                                           _digestDirectory;
	std::unique_ptr< Chain::ObjectStoreConnectionInterface >  _putConnection;
	DigestedTaskQueue                                         _putQueue;
	std::vector< std::thread >                                _threads;

	Hash putDirList()
	{
		std::ostringstream out;
		out << _digestDirectory.dirList();
		std::string text( out.str() );
		Hash hash = CalculateHash::SHA1( text.c_str(), text.size() );
		_putConnection->putString( text, hash );
		return hash;
	}

	static unsigned digestionThreads() { return numberOfCPUs() + 1; }

	CheckIn( const CheckIn & rhs ) = delete;
	CheckIn & operator= ( const CheckIn & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_H__
