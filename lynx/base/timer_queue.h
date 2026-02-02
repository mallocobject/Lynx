#ifndef LYNX_TIMER_QUEUE_H
#define LYNX_TIMER_QUEUE_H

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/base/timer_id.hpp"
#include "lynx/logger/logger.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>
namespace lynx
{
class Timer;
class EventLoop;
class Channel;
class TimerQueue
{
  private:
	using entry = std::pair<TimeStamp, Timer*>;

	int timer_fd_;
	EventLoop* loop_;
	std::unique_ptr<Channel> ch_;

	std::set<entry> timers_;
	std::vector<entry> active_timers_;
	std::set<Timer*>
		cancelled_timers_; // 没有合适的 entry 哈希算法 for unordered_set
	std::unordered_map<Timer*, entry> timer2entry_;

  public:
	DISABLE_COPY(TimerQueue)

	TimerQueue(EventLoop* loop);
	~TimerQueue();

	TimerId addTimer(TimeStamp time_stamp, const std::function<void()>& cb,
					 double interval);
	void cancell(TimerId timer_id);

  private:
	void readTimerFd()
	{
		uint64_t signal = 1;
		ssize_t n = ::read(timer_fd_, &signal, sizeof(signal));

		if (n != sizeof(signal))
		{
			LOG_ERROR << "EventLoop::readTimerFd() reads " << n
					  << " bytes instead of 8";
		}
	}

	void handleRead();
	void resetTimer();
	void resetTimerFd(Timer* timer);

	bool insert(Timer* timer);
	bool remove(Timer* timer);

	void addTimerInLocalLoop(Timer* timer);
	void cancellInLocalLoop(TimerId timer_id);
};
} // namespace lynx

#endif