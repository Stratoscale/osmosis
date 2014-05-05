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
		_producers( producers )
	{}

	void put( Task && task )
	{
		{
			std::lock_guard< std::mutex > lock( _mutex );
			_tasks.emplace( std::move( task ) );
		}
		_wait.notify_one();
	}

	void producerDone()
	{
		bool finished;
		{
			std::lock_guard< std::mutex > lock( _mutex );
			-- _producers;
			finished = _producers == 0;
		}
		if ( finished )
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
			return std::move( result );
		}
		if ( _producers == 0 )
			THROW( NoMoreTasksError, "No More Tasks" );
		else
			THROW( Error, "Internal assertion: should never be reached" );
	}

private:
	std::queue< Task >       _tasks;
	std::mutex               _mutex;
	std::condition_variable  _wait;
	unsigned                 _producers; 

	TaskQueue( const TaskQueue & rhs ) = delete;
	TaskQueue & operator= ( const TaskQueue & rhs ) = delete;
};

#endif // __COMMON_TASK_QUEUE_H__
