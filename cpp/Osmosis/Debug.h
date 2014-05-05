#ifndef __OSMOSIS_DEBUG_H__
#define __OSMOSIS_DEBUG_H__

#include "Common/Debug.h"
#include "Common/Exit.h"

#define CATCH_ALL( detailsSerialization ) \
	catch ( boost::exception & e ) { \
		TRACE_BOOST_EXCEPTION( e, detailsSerialization << \
				" on a boost exception, aborting" ); \
		exitWithError(); \
	} catch ( Error & e ) { \
		TRACE_ERROR( detailsSerialization << \
				" on 'Error' exception: '" \
				<< e.what() << "' at " << e.filename << ':' << e.line ); \
		exitWithError(); \
	} catch ( std::exception & e ) { \
		TRACE_ERROR( detailsSerialization << \
				" on std::exception: '" \
				<< e.what() << "'" ); \
		exitWithError(); \
	} catch ( ... ) { \
		TRACE_ERROR( detailsSerialization << \
				" on unknown exception" ); \
		exitWithError(); \
	}

#endif // __OSMOSIS_DEBUG_H__
