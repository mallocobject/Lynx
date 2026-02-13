#ifndef LYNX_TIMER_QUEUE_H
#define LYNX_TIMER_QUEUE_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.h"
#include "lynx/time/timer_id.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>
namespace lynx
{
class Timer;
class EventLoop;
class Channel;
class TimerQueue : public noncopyable
{
  private:
	using Entry = std::pair<TimeStamp, Timer*>;

	EventLoop* loop_;
	std::unique_ptr<Channel> ch_;

	std::set<Entry> timers_;
	std::vector<Entry> active_timers_;

  public:
	TimerQueue(EventLoop* loop);
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
	void resetTimerFd(Timer* timer);

	bool insert(Timer* timer);
	bool remove(Timer* timer);

	void addTimerInLoop(Timer* timer);
	void cancellInLoop(TimerId timer_id);
};
} // namespace lynx

#endif