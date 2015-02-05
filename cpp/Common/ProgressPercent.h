#ifndef __COMMON_PROGRESS_PERCENT_H__
#define __COMMON_PROGRESS_PERCENT_H__

class ProgressPercent
{
public:
	static unsigned calc(unsigned numerator, unsigned denominator, bool zeroIsDone = false)
	{
		if (denominator == 0)
			return zeroIsDone ? 100 : 0;
		return 100 * numerator / denominator;
	}
private:

	ProgressPercent( const ProgressPercent & rhs ) = delete;
	ProgressPercent & operator= ( const ProgressPercent & rhs ) = delete;
};

#endif // __COMMON_PROGRESS_PERCENT_H__
