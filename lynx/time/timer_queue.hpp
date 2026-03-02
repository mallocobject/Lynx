#ifndef LYNX_TIME_TIMER_QUEUE_HPP
#define LYNX_TIME_TIMER_QUEUE_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.hpp"
#include "lynx/time/timer_id.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>
namespace lynx
{
namespace tcp
{
class EventLoop;
class Channel;
} // namespace tcp

namespace time
{
class Timer;
class TimerQueue : public base::noncopyable
{
  private:
	using Entry = std::pair<TimeStamp, std::shared_ptr<Timer>>;

	tcp::EventLoop* loop_;
	std::unique_ptr<tcp::Channel> ch_;

	std::set<Entry> timers_;
	std::vector<Entry> active_timers_;

  public:
	TimerQueue(tcp::EventLoop* loop);
	~TimerQueue();

	void TimerDestroy();

	TimerId addTimer(TimeStamp time_stamp, const std::function<void()>& cb,
					 double interval);
	void cancell(TimerId timer_id);

	size_t size() const
	{
		return timers_.size();
	}

  private:
	void readTimerFd();
	void handleRead();
	void resetTimer();
	void resetTimerFd(std::shared_ptr<Timer> timer);

	bool insert(std::shared_ptr<Timer> timer);
	bool remove(std::shared_ptr<Timer> timer);

	void addTimerInLoop(std::shared_ptr<Timer> timer);
	void cancellInLoop(TimerId timer_id);
};
} // namespace time
} // namespace lynx

#endif