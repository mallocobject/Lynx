#ifndef LYNX_TIME_TIMER_ID_HPP
#define LYNX_TIME_TIMER_ID_HPP

#include <cstdint>
namespace lynx
{
namespace time
{
class Timer;
class TimerId
{
	friend class TimerQueue;

  private:
	Timer* timer_;
	int64_t seq_;

  public:
	TimerId() : timer_(nullptr), seq_(0)
	{
	}

	TimerId(Timer* timer, int64_t seq) : timer_(timer), seq_(seq)
	{
	}

	TimerId(const TimerId&) = default;
};
} // namespace time
} // namespace lynx

#endif