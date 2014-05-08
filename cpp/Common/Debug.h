#ifndef __COMMON_DEBUG_CPP_H__
#define __COMMON_DEBUG_CPP_H__

#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <boost/exception/diagnostic_information.hpp> 

#ifdef UNITTEST

//Under unittest: assert fails the test
#   define _ASSERT( __condition, __serialize ) do { \
	if ( ! ( __condition ) ) { \
		std::ostringstream __message; \
		__message << __serialize; \
		TS_FAIL( __message.str() ); \
	} \
    } while( 0 )

#else // UNITTEST

#   ifdef DEBUG

//Under debug build: assert exists immidiately
#       define _ASSERT( __condition, __serialize ) do { \
		if ( ! ( __condition ) ) { \
			_TRACE( "ASSERT", "Assertion '" #__condition "' failed (" << __FILE__ << ':' << \
					__LINE__ << ')' << __serialize << "\nCommiting suicide\n", std::cerr ); \
			_exit( 1 ); \
		} \
	} while( 0 )
#       define TRACE_DEVEL( __serialize ) _TRACE( "DEVEL", __serialize, std::cerr )
#       define TRACE_DEBUG( __serialize ) _TRACE( "DEBUG", __serialize, std::cout )

#   else //DEBUG

//Under release build: assert does not compile
#       define _ASSERT( __condition, __serialize ) do {} while( 0 )
#       define TRACE_DEVEL( __serialize ) please_dont_leave_TRACE_DEVEL_messages_lying_around
#       define TRACE_DEBUG( __serialize ) do {} while( 0 )

#   endif // DEBUG
#endif // UNITTEST

extern std::mutex globalTraceLock;
#   define _TRACE( __level, __serialize, __stream ) do { \
	struct timeval timeValue; \
	gettimeofday( & timeValue, nullptr ); \
	std::lock_guard< std::mutex > lock( globalTraceLock ); \
	__stream << timeValue.tv_sec << '.' << std::setfill( '0' ) << std::setw( 6 ) << timeValue.tv_usec << ':' << \
		__level << ':' << ' ' << __serialize << ' ' << '(' << __FILE__ << ':' << __LINE__ << \
		')' << std::endl; \
    } while( 0 )

#define TRACE_INFO( __serialize ) _TRACE( "INFO", __serialize, std::cout )
#define TRACE_WARNING( __serialize ) _TRACE( "WARNING", __serialize, std::cerr )
#define TRACE_ERROR( __serialize ) _TRACE( "ERROR", __serialize, std::cerr )
#define TRACE_EXCEPTION( __e, __serialize ) _TRACE( "EXCEPTION", "Exception: " << __e.what() << ": " << __serialize, std::cerr )
#define ASSERT( __x ) _ASSERT( __x, "" )
#define ASSERT_VERBOSE( __x, __serialize ) _ASSERT( __x, ": " << __serialize )

#define TRACE_BOOST_EXCEPTION( e, serialize ) _TRACE( "EXCEPTION", "Boost Exception: " << boost::diagnostic_information( e ) << ": " << serialize, std::cerr )
#define THROW_BOOST_ERRNO_EXCEPTION( errno, serialize ) do { \
	std::ostringstream message; \
	message << serialize; \
	BOOST_THROW_EXCEPTION( \
		boost::system::system_error( \
			boost::system::error_code( errno, \
			boost::system::generic_category() ), \
			message.str() ) ); \
	} while( 0 )

#endif // __COMMON_DEBUG_H__
//FILE_EXEMPT_FROM_CODE_COVERAGE
