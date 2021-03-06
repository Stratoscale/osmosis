#ifndef __OSMOSIS_DEBUG_H__
#define __OSMOSIS_DEBUG_H__

#include "Common/Debug.h"
#include "Common/Exit.h"

#define CATCH_ALL( detailsSerialization, __code ) \
	catch ( boost::exception & e ) { \
		TRACE_BOOST_EXCEPTION( e, detailsSerialization << \
				" on a boost exception" ); \
		__code \
	} catch ( Error & e ) { \
		TRACE_ERROR( detailsSerialization << \
				" on 'Error' exception: '" \
				<< e.what() << "' from " << e.backtrace() ); \
		__code \
	} catch ( std::exception & e ) { \
		TRACE_ERROR( detailsSerialization << \
				" on std::exception: '" \
				<< e.what() << "'" ); \
		__code \
	} catch ( ... ) { \
		TRACE_ERROR( detailsSerialization << \
				" on unknown exception" ); \
		__code \
	}

#define CATCH_ALL_SUICIDE( detailsSerialization ) \
	CATCH_ALL( detailsSerialization, exitWithError(); )
#define CATCH_ALL_IGNORE( detailsSerialization ) \
	CATCH_ALL( detailsSerialization, )

#define CATCH_TRACEBACK_EXCEPTION CATCH_ALL( "From here", throw; )

#endif // __OSMOSIS_DEBUG_H__
