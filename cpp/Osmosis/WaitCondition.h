#ifndef __WAIT_CONDITION_H__
#define __WAIT_CONDITION_H__

#include <mutex>
#include <condition_variable>

class WaitCondition
{
public:
	WaitCondition( unsigned sleepInterval );

	void stop();

	bool sleep();

private:
	unsigned                  _sleepInterval;
	std::mutex                _stopLock;
	std::condition_variable   _stop;
	bool                      _stopped;

	WaitCondition( const WaitCondition & rhs ) = delete;
	WaitCondition & operator= ( const WaitCondition & rhs ) = delete;
};

#endif // __WAIT_CONDITION_H__
