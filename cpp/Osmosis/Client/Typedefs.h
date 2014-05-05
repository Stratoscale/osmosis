#ifndef __OSMOSIS_CLIENT_TYPEDEFS_H__
#define __OSMOSIS_CLIENT_TYPEDEFS_H__

#include "Common/TaskQueue.h"
#include "Osmosis/Client/Digested.h"
#include "Osmosis/Client/ToVerify.h"

namespace Osmosis {
namespace Client
{

typedef TaskQueue< boost::filesystem::path > PathTaskQueue;
typedef TaskQueue< struct Digested > DigestedTaskQueue;
typedef TaskQueue< struct ToVerify > ToVerifyTaskQueue;

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_TYPEDEFS_H__
