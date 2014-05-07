#ifndef __OSMOSIS_CLIENT_TO_VERIFY_H__
#define __OSMOSIS_CLIENT_TO_VERIFY_H__

#include "Osmosis/FileStatus.h"

namespace Osmosis {
namespace Client
{

struct ToVerify
{
	boost::filesystem::path  path;
	FileStatus               status;
	Hash                     hash;
	boost::filesystem::path  draft;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_TO_VERIFY_H__
