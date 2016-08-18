#ifndef __OSMOSIS_CLIENT_DIGESTED_H__
#define __OSMOSIS_CLIENT_DIGESTED_H__

#include "Osmosis/Hash.h"

namespace Osmosis {
namespace Client
{

struct Digested
{
	boost::filesystem::path path;
	Hash hash;
};

} // namespace Client
} // namespace Osmosis

#endif // __OSMOSIS_CLIENT_DIGESTED_H__
