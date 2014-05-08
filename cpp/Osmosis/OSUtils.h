#ifndef __OSMOSIS_OSUTILS_H__
#define __OSMOSIS_OSUTILS_H__

namespace Osmosis {
namespace OSUtils
{

static inline uid_t uid()
{
	static uid_t uid = getuid();
	return uid;
}

static inline gid_t gid()
{
	static gid_t gid = getgid();
	return gid;
}

} // namespace OSUtils
} // namespace Osmosis

#endif // __OSMOSIS_OSUTILS_H__
