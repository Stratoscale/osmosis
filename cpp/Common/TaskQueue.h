#ifndef __COMMON_TASK_QUEUE_H__
#define __COMMON_TASK_QUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Common/Error.h"
#include "Common/Debug.h"

template < class Task >
class TaskQueue
{
public:
	TaskQueue( unsigned producers ) :
		_producers( producers ),
		_putCount( 0 ),
		_getCount( 0 )
	{}

	size_t size() const
	{
		std::lock_guard< std::mutex > lock( _mutex );
		return _tasks.size();
	}

	void put( Task && task )
	{
		{
			std::lock_guard< std::mutex > lock( _mutex );
			_putCount += 1;
			_tasks.emplace( std::move( task ) );
		}
		_wait.notify_one();
	}

	void producerDone()
	{
		bool finished;
		{
			std::lock_guard< std::mutex > lock( _mutex );
			if ( _producers == 0 )
				return;
			-- _producers;
			finished = _producers == 0;
		}
		if ( finished )
			_wait.notify_all();
	}

	void abort()
	{
		{
			std::lock_guard< std::mutex > lock( _mutex );
			if ( _producers == 0 )
				return;
			_producers = 0;
		}
		_wait.notify_all();
	}

	EXCEPTION_CLASS( NoMoreTasksError );

	Task get()
	{
		std::unique_lock< std::mutex > lock( _mutex );
		while ( _producers > 0 and _tasks.empty() )
			_wait.wait( lock );
		if ( not _tasks.empty() ) {
			ASSERT( not _tasks.empty() );
			Task result( std::move( _tasks.front() ) );
			_tasks.pop();
			_getCount += 1;
			return result;
		}
		if ( _producers == 0 )
			THROW( NoMoreTasksError, "No More Tasks" );
		else
			THROW( Error, "Internal assertion: should never be reached" );
	}

	unsigned putCount() const { return _putCount; }
	unsigned getCount() const { return _getCount; }

private:
	std::queue< Task >       _tasks;
	mutable std::mutex       _mutex;
	std::condition_variable  _wait;
	unsigned                 _producers;
	unsigned                 _putCount;
	unsigned                 _getCount;

	TaskQueue( const TaskQueue & rhs ) = delete;
	TaskQueue & operator= ( const TaskQueue & rhs ) = delete;
};

#endif // __COMMON_TASK_QUEUE_H__
