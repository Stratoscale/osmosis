#ifndef __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__
#define __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Client/FetchFiles.h"
#include "Osmosis/Stream/BufferToSocket.h"
#include "Osmosis/WaitCondition.h"

namespace Osmosis {
namespace Client
{

class CheckOutProgress
{
public:
	CheckOutProgress(       const boost::filesystem::path &  path,
				DigestDirectory &                digestDirectory,
				unsigned                         reportIntervalSeconds );

	~CheckOutProgress();

	void stop();

	void setFetchFiles( FetchFiles & fetchFiles );

private:
	const boost::filesystem::path  _path;
	DigestDirectory &              _digestDirectory;
	FetchFiles *                   _fetchFiles;
	std::thread                    _thread;
	WaitCondition                  _threadStopCondition;

	void threadEntryPoint();

	void report( bool zeroIsDone );

	CheckOutProgress( const CheckOutProgress & rhs ) = delete;
	CheckOutProgress & operator= ( const CheckOutProgress & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_OUT_PROGRESS_H__
