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

#define EXCEPTION_SUBCLASS( name, superclass ) \
	class name : public superclass \
	{ \
	public: \
		using ::Error::Error; \
	}

#define EXCEPTION_CLASS( name ) EXCEPTION_SUBCLASS( name, ::Error )

#define THROW( name, serialize ) do { \
		std::ostringstream __serialize; \
		__serialize << serialize; \
		throw name( __serialize.str(), __FILE__, __LINE__ ); \
	} while( 0 )

#endif // __COMMON_ERROR_H__
//FILE_EXEMPT_FROM_CODE_COVERAGE
