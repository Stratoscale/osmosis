#ifndef __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
#define __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__

#include "Osmosis/Client/DigestDirectory.h"
#include "Osmosis/Client/CheckExistingThread.h"
#include "Osmosis/Client/PutThread.h"
#include "Osmosis/Stream/BufferToSocket.h"
#include "Common/NumberOfCPUs.h"

namespace Osmosis {
namespace Client
{

class CheckInProgress
{
public:
	CheckInProgress( const boost::filesystem::path & path ):
		_path( path ),
		_enabled( path != boost::filesystem::path( "" ) )
	{}

	void update

private:
        const boost::filesystem::path  _path;
	bool                           _enabled;

	CheckInProgress( const CheckInProgress & rhs ) = delete;
	CheckInProgress & operator= ( const CheckInProgress & rhs ) = delete;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_CHECK_IN_PROGRESS_H__
