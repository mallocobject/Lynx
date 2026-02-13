#include "lynx/time/timer_queue.h"
#include "lynx/logger/logger.h"
#include "lynx/tcp/channel.h"
#include "lynx/tcp/event_loop.h"
#include "lynx/time/time_stamp.h"
#include "lynx/time/timer.h"
#include "lynx/time/timer_id.hpp"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <memory>
#include <sys/timerfd.h>

using namespace lynx;

TimerQueue::TimerQueue(EventLoop* loop) : loop_(loop)
{
	int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	ch_ = std::make_unique<Channel>(fd, loop);
	ch_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
	ch_->enableIN();
}

TimerQueue::~TimerQueue()
{
	ch_->disableAll();
	ch_->remove();
	for (auto& entry : timers_)
	{
		delete entry.second;
	}
}

TimerId TimerQueue::addTimer(TimeStamp time_stamp,
							 const std::function<void()>& cb, double interval)
{
	Timer* timer = new Timer(time_stamp, cb, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));

	return TimerId(timer, timer->seq());
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();

	if (insert(timer))
	{
		resetTimerFd(timer);
	}

	timer->setOn(true);
}

void TimerQueue::cancell(TimerId timer_id)
{
	loop_->runInLoop(std::bind(&TimerQueue::cancellInLoop, this, timer_id));
}

void TimerQueue::cancellInLoop(TimerId timer_id)
{
	loop_->assertInLoopThread();

	Timer* timer = timer_id.timer_;

	if (!timer)
	{
		LOG_WARN << "timer is null";
		return;
	}

	if (timer->seq() != timer_id.seq_)
	{
		LOG_WARN << "sequence mismatch, timer "
					"might have been reset";
		return;
	}

	timer->setOn(false);
}

void TimerQueue::readTimerFd()
{
	uint64_t signal = 1;
	ssize_t n = ::read(ch_->fd(), &signal, sizeof(signal));

	if (n != sizeof(signal))
	{
		LOG_ERROR << "Timer fd reads " << n << " bytes instead of 8";
	}
}
void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();

	readTimerFd();
	active_timers_.clear();

	auto end = timers_.upper_bound(
		Entry(TimeStamp::now(), reinterpret_cast<Timer*>(UINTPTR_MAX)));
	active_timers_.insert(active_timers_.end(), timers_.begin(), end);

	timers_.erase(timers_.begin(), end);
	for (auto& entry : active_timers_)
	{
		Timer* timer = entry.second;

		if (timer->on())
		{
			timer->run();
		}
	}
	resetTimer(); // sync
}

void TimerQueue::resetTimer()
{
	for (auto& entry : active_timers_)
	{
		Timer* timer = entry.second;

		if (!timer->on())
		{
			delete timer;
			timer = nullptr;
			continue;
		}

		if (timer->repeating())
		{
			timer->repeat();
			insert(timer);
		}
		else
		{
			delete timer;
			timer = nullptr;
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
		timer->expiration().microseconds() - TimeStamp::now().microseconds();
	if (micro_seconds_diff < 100)
	{
		micro_seconds_diff = 100;
	}

	value.it_value.tv_sec =
		static_cast<time_t>(micro_seconds_diff / kMicroSecond2Second);
	// 纳秒
	value.it_value.tv_nsec =
		static_cast<long>(micro_seconds_diff % kMicroSecond2Second) * 1000;

	int ret = ::timerfd_settime(ch_->fd(), 0, &value, nullptr);
	assert(ret != -1);
	if (ret == -1)
	{
		LOG_ERROR << "time setting faild - fd: " << ch_->fd() << " : "
				  << strerror(errno);
	}
}

bool TimerQueue::insert(Timer* timer)
{
	bool reset_instantly = false;

	if (timers_.empty() || timer->expiration() < timers_.begin()->first)
	{
		reset_instantly = true;
	}

	Entry e(timer->expiration(), timer);

	timers_.insert(e);

	return reset_instantly;
}
