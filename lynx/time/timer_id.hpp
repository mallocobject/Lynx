#ifndef LYNX_TIME_TIMER_ID_HPP
#define LYNX_TIME_TIMER_ID_HPP

#include <cstdint>
#include <memory>
namespace lynx
{
namespace time
{
class Timer;
class TimerId
{
	// friend class TimerQueue;

  private:
	std::weak_ptr<Timer> timer_;
	uint64_t id_;

  public:
	TimerId() : id_(0)
	{
	}

	TimerId(std::weak_ptr<Timer> timer, uint64_t id) : timer_(timer), id_(id)
	{
	}

	TimerId(const TimerId&) = default;

	bool isAlive() const
	{
		return !timer_.expired();
	}

	std::shared_ptr<Timer> timer() const
	{
		return timer_.lock();
	}

	uint64_t id() const
	{
		return id_;
	}
};
} // namespace time
} // namespace lynx

#endif