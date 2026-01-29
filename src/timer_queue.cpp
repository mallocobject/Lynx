#include "lynx/include/timer_queue.h"
#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include "lynx/include/time_stamp.h"
#include "lynx/include/timer.h"
#include "lynx/include/timer_id.hpp"
#include <cassert>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>
#include <strings.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <utility>

namespace lynx
{
TimerQueue::TimerQueue(EventLoop* loop)
	: loop_(loop),
	  timer_fd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
	  ch_(std::make_unique<Channel>(timer_fd_, loop))
{
	ch_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
	ch_->enableIN();
}

TimerQueue::~TimerQueue()
{
	ch_->disableAll();
	ch_->remove();
	for (auto& p : timers_)
	{
		delete p.second;
	}
}

TimerId TimerQueue::addTimer(TimeStamp time_stamp,
							 const std::function<void()>& cb, double interval)
{
	Timer* timer = new Timer(time_stamp, cb, interval);
	loop_->runInLocalThread(
		std::bind(&TimerQueue::addTimerInLocalLoop, this, timer));
	return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLocalLoop(Timer* timer)
{
	loop_->assertInLocalThread();

	if (insert(timer))
	{
		resetTimerFd(timer);
	}
}

void TimerQueue::cancell(TimerId timer_id)
{
	loop_->runInLocalThread(
		std::bind(&TimerQueue::cancellInLocalLoop, this, timer_id));
}

void TimerQueue::cancellInLocalLoop(TimerId timer_id)
{
	loop_->assertInLocalThread();

	Timer* timer = timer_id.timer_;
	if (!timer)
	{
		LOG_WARN << "TimerQueue::cancellInLocalLoop - timer is null";
		return;
	}

	if (timer->sequence() != timer_id.sequence_)
	{
		LOG_WARN << "TimerQueue::cancellInLocalLoop - sequence mismatch, timer "
					"might have been reset";
		return;
	}

	auto it = timer2entry_.find(timer);
	if (it == timer2entry_.end())
	{
		LOG_DEBUG << "TimerQueue::cancellInLocalLoop - timer not found, might "
					 "already be "
					 "triggered or cancelled";
		return;
	}

	cancelled_timers_.insert(timer);
	if (remove(timer))
	{
		if (!timers_.empty())
		{
			resetTimerFd(timers_.begin()->second);
		}
	}
	LOG_INFO << "TimerQueue::cancellInLocalLoop - timer cancelled successfully";
}

void TimerQueue::handleRead()
{
	loop_->assertInLocalThread();

	readTimerFd();
	active_timers_.clear();

	auto end = timers_.upper_bound(
		entry(TimeStamp::now(), reinterpret_cast<Timer*>(UINTPTR_MAX)));
	active_timers_.insert(active_timers_.end(), timers_.begin(), end);

	timers_.erase(timers_.begin(), end);
	for (auto& p : active_timers_)
	{
		p.second->run();
	}
	resetTimer();
}

void TimerQueue::resetTimer()
{
	for (auto& p : active_timers_)
	{
		auto timer = p.second;
		if (cancelled_timers_.find(timer) != cancelled_timers_.end())
		{
			cancelled_timers_.erase(timer);
			delete timer;
			continue;
		}

		if (timer->repeat())
		{
			timer->reset();
			insert(timer);
		}
		else
		{
			delete timer;
		}
	}

	if (!timers_.empty())
	{
		resetTimerFd(timers_.begin()->second);
	}
}

void TimerQueue::resetTimerFd(Timer* timer)
{
	itimerspec value;
	::bzero(&value, sizeof(value));

	int64_t micro_seconds_diff =
		timer->expiration().microSeconds() - TimeStamp::now().microSeconds();
	if (micro_seconds_diff < 100)
	{
		micro_seconds_diff = 100;
	}

	value.it_value.tv_sec =
		static_cast<time_t>(micro_seconds_diff / MicroSecond2Second);
	// 纳秒
	value.it_value.tv_nsec =
		static_cast<long>(micro_seconds_diff % MicroSecond2Second) * 1000;

	int ret = ::timerfd_settime(timer_fd_, 0, &value, nullptr);
	assert(ret != -1);
}

bool TimerQueue::insert(Timer* timer)
{
	bool reset_instantly = false;
	if (cancelled_timers_.find(timer) != cancelled_timers_.end())
	{
		cancelled_timers_.erase(timer);
		delete timer;
		return false;
	}
	if (timers_.empty() || timer->expiration() < timers_.begin()->first)
	{
		reset_instantly = true;
	}

	entry e(timer->expiration(), timer);

	timers_.insert(e);

	timer2entry_[timer] = e;

	return reset_instantly;
}

// 如果成功删除就返回 true，意味着要调整 定时器唤醒时间
bool TimerQueue::remove(Timer* timer)
{
	auto it = timer2entry_.find(timer);
	if (it == timer2entry_.end())
	{
		return false;
	}

	size_t n = timers_.erase(it->second);
	timer2entry_.erase(it);

	return n > 0;
}
} // namespace lynx