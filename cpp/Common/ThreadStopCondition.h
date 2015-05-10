#ifndef __COMMON_THREAD_STOP_CONDITION_H__
#define __COMMON_THREAD_STOP_CONDITION_H__

#include <mutex>

class ThreadStopCondition
{
public:
	ThreadStopCondition( unsigned sleepInterval ) :
		_sleepInterval( sleepInterval ),
		_stopped( false )
	{}

	void stop()
	{
		{
			std::unique_lock< std::mutex > lock( _stopLock );
			_stopped = true;
		}
		_stop.notify_one();
	}

	bool sleep()
	{
		std::unique_lock< std::mutex > lock( _stopLock );
		if ( _stopped )
			return false;
		_stop.wait_for( lock, std::chrono::seconds( _sleepInterval ) );
		return not _stopped;
	}

private:
	unsigned                  _sleepInterval;
	std::mutex                _stopLock;
	std::condition_variable   _stop;
	bool                      _stopped;

	ThreadStopCondition( const ThreadStopCondition & rhs ) = delete;
	ThreadStopCondition & operator= ( const ThreadStopCondition & rhs ) = delete;
};

#endif // __COMMON_THREAD_STOP_CONDITION_H__
