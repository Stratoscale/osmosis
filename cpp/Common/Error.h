#ifndef __COMMON_ERROR_H__
#define __COMMON_ERROR_H__

#include <string>
#include <stdexcept>
#include <sstream>
#include "Common/Debug.h"

class Error : public std::runtime_error
{
public:
	Error( const std::string & what, const char * filename, unsigned line ) :
		std::runtime_error( what ),
		backtraceLength( 1 )
	{
		backtraceEntries[ 0 ].filename = filename;
		backtraceEntries[ 0 ].line = line;
	}

	struct Backtrace {
		const char *  filename;
		unsigned      line;
		std::string   message;
	};
	enum { MAXIMUM_BACKTRACE = 10 };
	struct Backtrace backtraceEntries[ MAXIMUM_BACKTRACE ];
	unsigned backtraceLength;

	void addBacktrace( const char * filename, unsigned line, const std::string * message = nullptr )
	{
		if ( backtraceLength == MAXIMUM_BACKTRACE )
			return;
		backtraceEntries[ backtraceLength ].filename = filename;
		backtraceEntries[ backtraceLength ].line = line;
		if ( message != nullptr )
			backtraceEntries[ backtraceLength ].message = std::move( * message );
		++ backtraceLength;
	}

	std::string backtrace() const
	{
		std::ostringstream result;
		ASSERT( backtraceLength > 0 );
		for ( int i = backtraceLength - 1; i >= 0; -- i ) {
			auto & entry = backtraceEntries[ i ];
			if ( i != (int)backtraceLength - 1 )
				result << " -> ";
			result << entry.filename << ':' << entry.line;
			if ( entry.message.size() > 0 )
				result << " \"" << entry.message << '"';
		}
		return std::move(result.str());

	}
};


class LabelFileIsCorrupted final : public Error
{
public:
	LabelFileIsCorrupted( const std::string & what, const char * filename, unsigned line ) :
		Error( what, filename, line )
	{
	}
};

#define EXCEPTION_SUBCLASS( __name, __superclass ) \
	class __name : public __superclass \
	{ \
	public: \
		using ::Error::Error; \
	}

#define EXCEPTION_CLASS( __name ) EXCEPTION_SUBCLASS( __name, ::Error )

#define THROW( __name, __serialize ) do { \
		std::ostringstream __serialized; \
		__serialized << __serialize; \
		throw __name( __serialized.str(), __FILE__, __LINE__ ); \
	} while( 0 )

#define BACKTRACE_BEGIN \
	try {
#define BACKTRACE_END \
	} catch ( ::Error & e ) { \
		e.addBacktrace( __FILE__, __LINE__ ); \
		throw; \
	} catch ( boost::exception & ) { \
		TRACE_ERROR( "Backtrace: from here" ); \
		throw; \
	}
#define BACKTRACE_END_VERBOSE( __serialize ) \
	} catch ( ::Error & e ) { \
		std::ostringstream __serialized; \
		__serialized << __serialize; \
		std::string __asString = std::move( __serialized.str() ); \
		e.addBacktrace( __FILE__, __LINE__, & __asString ); \
		throw; \
	} catch ( boost::exception & ) { \
		TRACE_ERROR( "Backtrace: from here:" << __serialize ); \
		throw; \
	}

#endif // __COMMON_ERROR_H__
//FILE_EXEMPT_FROM_CODE_COVERAGE
