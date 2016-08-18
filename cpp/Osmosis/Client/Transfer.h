#ifndef __OSMOSIS_CLIENT_TRANSFER_H__
#define __OSMOSIS_CLIENT_TRANSFER_H__

#include <thread>
#include "Osmosis/Client/TransferThread.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/Typedefs.h"
#include "Osmosis/DirList.h"

namespace Osmosis {
namespace Client
{

class Transfer
{
public:
	Transfer(       const std::string &            label,
			Chain::Chain &                 chain,
			Chain::ObjectStoreInterface &  destination );

	~Transfer();

	void go();

private:
	enum {
		CHECK_EXISTING_THREADS = 10,
		TRANSFER_THREADS = 5,
	};

	const std::string                      _label;
	Chain::Chain &                         _chain;
	Chain::ObjectStoreInterface &          _destination;
	CheckExistingThread::AlreadyProcessed  _checkExistingAlreadyProcessed;
	std::mutex                             _checkExistingAlreadyProcessedLock;
	DigestedTaskQueue                      _toCheckExistingQueue;
	DigestedTaskQueue                      _toTransferQueue;
	std::vector< std::thread >             _threads;

	void checkLabelDoesNotExistInDestination( Chain::ObjectStoreConnectionInterface & connection );

	void populateToCheckExistingQueue( const Hash & labelHash, const DirList & labelDirList );

	DirList fetchDirList( const Hash & labelHash, Chain::CheckOut & checkOut );

	Transfer( const Transfer & rhs ) = delete;
	Transfer & operator= ( const Transfer & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_TRANSFER_H__
