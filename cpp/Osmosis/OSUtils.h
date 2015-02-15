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

static inline pid_t pid()
{
	static pid_t pid = getpid();
	return pid;
}

} // namespace OSUtils
} // namespace Osmosis

#endif // __OSMOSIS_OSUTILS_H__
