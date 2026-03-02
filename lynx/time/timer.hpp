#ifndef LYNX_TIME_TIMER_HPP
#define LYNX_TIME_TIMER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
namespace lynx
{
namespace time
{
class Timer : public base::noncopyable
{
  private:
	TimeStamp expiration_;
	double interval_;
	bool repeating_;
	static std::atomic<uint64_t> id_creator_;
	// int64_t id_;

	bool on_;

	std::function<void()> callback_;

  public:
	Timer(TimeStamp expiration, std::function<void()> cb, double interval);
	~Timer();

	bool repeating() const
	{
		return repeating_;
	}

	// int64_t seq() const
	// {
	// 	return seq_;
	// }

	static uint64_t generateId()
	{
		return id_creator_.fetch_add(1, std::memory_order_acq_rel);
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
} // namespace time
} // namespace lynx

#endif