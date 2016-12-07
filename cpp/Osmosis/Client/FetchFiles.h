#ifndef __OSMOSIS_CLIENT_FETCH_FILES_H__
#define __OSMOSIS_CLIENT_FETCH_FILES_H__

#include "Osmosis/Chain/Chain.h"
#include "Osmosis/Client/DigestDrafts.h"

namespace Osmosis {
namespace Client
{

class FetchFiles
{
public:
	FetchFiles( const boost::filesystem::path & directory, Chain::Chain & chain, Chain::CheckOut & chainCheckOut );

	~FetchFiles();

	void join();

	void noMoreFilesToFetch();

	void fetch( const boost::filesystem::path & path, const FileStatus & status, const Hash & hash );

	boost::filesystem::path draftsPath() const;

	struct Stats {
		unsigned fetchesRequested;
		unsigned fetchesCompleted;
		unsigned digestQueueLength;
	};

	Stats stats();

	const Chain::CheckOut::GetCountStats & checkOutGetCount();

private:
	const boost::filesystem::path  _directory;
	Chain::CheckOut &              _checkOut;
	ObjectStore::Drafts            _drafts;
	DigestDrafts                   _digestDrafts;
	ToVerifyTaskQueue              _fetchQueue;
	std::vector< std::thread >     _threads;
	bool                           _done;
	unsigned                       _fetchRequested;
	unsigned                       _fetchCompleted;

	void fetchThreadEntryPoint();

	void fetchOne();

	void afterVerifiedThreadEntryPoint();

	void commitOne();

	void possiblySignalProducerDone();
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_FETCH_FILES_H__
