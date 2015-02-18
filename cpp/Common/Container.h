#ifndef __COMMON_CONTAINER_H__
#define __COMMON_CONTAINER_H__

#include "Common/Debug.h"

template< typename T >
class Container
{
public:
	Container() : _constructed( false )
	{
		ASSERT( HOW_MANY_UNSIGNEDS_COVER_SIZE_T * sizeof( unsigned ) >= sizeof( T ) );
	}

	~Container()
	{
		if ( _constructed )
			destroy();
	}

	void destroy()
	{
		ASSERT( _constructed );
		_constructed = false;
		( * this )->~T();
	}

	template< typename... Args >
	void emplace( Args && ... args )
	{
		ASSERT( ! _constructed );
		new ( (void *) _data ) T( std::forward< Args >( args )... );
		_constructed = true;
	}

	bool constructed() const { return _constructed; }

	T & operator* () { return * (T *) (void *) _data; }
	const T & operator* () const { return * (T *) (void *) _data; }
	T * operator-> () { return (T *) (void *) _data; }
	const T * operator-> () const { return (T *) (void *) _data; }

private:
	enum { HOW_MANY_UNSIGNEDS_COVER_SIZE_T = ( sizeof( T ) + sizeof( unsigned ) - 1 ) / sizeof( unsigned ) };

	unsigned	_data[ HOW_MANY_UNSIGNEDS_COVER_SIZE_T ];
	bool		_constructed;
};

#endif // __COMMON_CONTAINER_H__
