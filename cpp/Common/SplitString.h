#ifndef __COMMON_SPLIT_STRING_H__
#define __COMMON_SPLIT_STRING_H__

#include "Common/Debug.h"

class SplitString
{
public:
	SplitString( const std::string & data, char delimiter );

	bool done() const;

	void next();

	std::string asString() const;

	const char * asCharPtr() const;

	unsigned charCount() const;

private:
	const std::string & _data;
	char _delimiter;
	size_t _start;
	size_t _end;

	SplitString( const SplitString & rhs ) = delete;
	SplitString & operator= ( const SplitString & rhs ) = delete;
};

#endif // __COMMON_SPLIT_STRING_H__
