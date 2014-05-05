#ifndef __COMMON_NUMBER_OF_CPUS_H__
#define __COMMON_NUMBER_OF_CPUS_H__

#include <unistd.h>

static inline unsigned numberOfCPUs()
{
	int result = sysconf( _SC_NPROCESSORS_ONLN );
	if ( result < 0 )
		THROW_BOOST_ERRNO_EXCEPTION( errno, "Unable to fetch number of processors" );
	return result;
}

#endif // __COMMON_NUMBER_OF_CPUS_H__
