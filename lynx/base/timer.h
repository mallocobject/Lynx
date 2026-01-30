#ifndef LYNX_TIMER_H
#define LYNX_TIMER_H

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include <atomic>
#include <cstdint>
#include <functional>

namespace lynx
{
class Timer
{
  private:
	TimeStamp expiration_;
	double interval_;
	bool repeat_;
	static std::atomic<int64_t> seq_creator_;
	int64_t sequence_;

	std::function<void()> callback_;

  public:
	DISABLE_COPY(Timer)

	// repeatly call function if interval > 0
	Timer(TimeStamp expiration, std::function<void()> cb, double interval);
	~Timer() = default;

	bool repeat() const
	{
		return repeat_;
	}

	int64_t sequence() const
	{
		return sequence_;
	}

	static int64_t seq_created()
	{
		return seq_creator_;
	}

	TimeStamp expiration() const
	{
		return expiration_;
	}

	void run() const
	{
		callback_();
	}

	void reset()
	{
		expiration_ = TimeStamp::addTime(TimeStamp::now(), interval_);
	}
};
} // namespace lynx

#endif