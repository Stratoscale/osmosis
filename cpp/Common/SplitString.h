#ifndef __COMMON_SPLIT_STRING_H__
#define __COMMON_SPLIT_STRING_H__

class SplitString
{
public:
	SplitString( const std::string & data, char delimiter ) :
		_data( data ),
		_delimiter( delimiter ),
		_start( 0 )
	{
		_end = _data.find_first_of( _delimiter, _start );
	}

	bool done() const { return _start == std::string::npos; }

	void next()
	{
		ASSERT( not done() );
		if ( _end == std::string::npos ) {
			_start = std::string::npos;
			return;
		}
		_start = _end + 1;
		_end = _data.find_first_of( _delimiter, _start );
	}

	std::string asString() const
	{
		ASSERT( not done() );
		std::string result;
		if ( _end == std::string::npos )
			result = _data.substr( _start );
		else
			result = _data.substr( _start, _end - _start );
		return std::move( result );
	}

private:
	const std::string & _data;
	char _delimiter;
	size_t _start;
	size_t _end;

	SplitString( const SplitString & rhs ) = delete;
	SplitString & operator= ( const SplitString & rhs ) = delete;
};

#endif // __COMMON_SPLIT_STRING_H__
