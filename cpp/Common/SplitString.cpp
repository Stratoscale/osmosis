#include <string>
#include "Common/SplitString.h"
#include "Common/Debug.h"

SplitString::SplitString( const std::string & data, char delimiter ) :
	_data( data ),
	_delimiter( delimiter ),
	_start( 0 )
{
	_end = _data.find_first_of( _delimiter, _start );
}

bool SplitString::done() const { return _start == std::string::npos; }

void SplitString::next()
{
	ASSERT( not done() );
	if ( _end == std::string::npos ) {
		_start = std::string::npos;
		return;
	}
	_start = _end + 1;
	_end = _data.find_first_of( _delimiter, _start );
}

std::string SplitString::asString() const
{
	ASSERT( not done() );
	std::string result;
	if ( _end == std::string::npos )
		result = _data.substr( _start );
	else
		result = _data.substr( _start, _end - _start );
	return result;
}

const char * SplitString::asCharPtr() const
{
	ASSERT( not done() );
	return _data.c_str() + _start;
}

unsigned SplitString::charCount() const
{
	ASSERT( not done() );
	if ( _end == std::string::npos )
		return _data.size() - _start;
	else
		return _end - _start;
}
