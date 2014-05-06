#ifndef __COMMON_ERROR_H__
#define __COMMON_ERROR_H__

#include <string>
#include <stdexcept>
#include <sstream>

class Error : public std::runtime_error
{
public:
	Error( const std::string & what, const char * filename, unsigned line ) :
		std::runtime_error( what ),
		filename( filename ),
		line( line )
	{}

	const char * const filename;
	const unsigned line;
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

#endif // __COMMON_ERROR_H__
//FILE_EXEMPT_FROM_CODE_COVERAGE
