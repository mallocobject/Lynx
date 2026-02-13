#ifndef LYNX_TIMER_H
#define LYNX_TIMER_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.h"
#include <atomic>
#include <cstdint>
#include <functional>
namespace lynx
{
class Timer : public noncopyable
{
  private:
	TimeStamp expiration_;
	double interval_;
	bool repeating_;
	static std::atomic<int64_t> seq_creator_;
	int64_t seq_;

	bool on_;

	std::function<void()> callback_;

  public:
	Timer(TimeStamp expiration, std::function<void()> cb, double interval);
	~Timer();

	bool repeating() const
	{
		return repeating_;
	}

	int64_t seq() const
	{
		return seq_;
	}

	void setOn(bool on = false)
	{
		on_ = on;
	}

	bool on() const
	{
		return on_;
	}

	TimeStamp expiration() const
	{
		return expiration_;
	}

	void run() const
	{
		callback_();
	}

	void repeat()
	{
		expiration_ = TimeStamp::addTime(TimeStamp::now(), interval_);
	}
};
} // namespace lynx

#endif