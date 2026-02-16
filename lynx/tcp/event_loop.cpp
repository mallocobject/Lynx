#include "lynx/tcp/event_loop.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/channel.hpp"
#include "lynx/tcp/epoller.hpp"
#include "lynx/time/timer_id.hpp"
#include "lynx/time/timer_queue.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <sys/eventfd.h>

using namespace lynx;

EventLoop::EventLoop()
	: epoller_(std::make_unique<Epoller>()), tid_(CurrentThread::tid()),
	  quit_(true), calling_pending_funcs_(false)
{
	tq_ = std::make_unique<TimerQueue>(this);

	wakeup_fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (wakeup_fd_ == -1)
	{
		LOG_FATAL << "eventfd failed: " << ::strerror(errno);
	}

	wakeup_ch_ = std::make_unique<Channel>(wakeup_fd_, this);
	wakeup_ch_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	wakeup_ch_->enableIN();

	LOG_TRACE << "EventLoop created " << this << " in thread " << tid_;
}

EventLoop::~EventLoop()
{
}

void EventLoop::updateChannel(Channel* ch)
{
	epoller_->updataChannel(ch);
}

void EventLoop::removeChannel(Channel* ch)
{
	epoller_->removeChannel(ch);
}

void EventLoop::run()
{
	LOG_TRACE << "EventLoop " << this << " start looping";
	assert(quit_.load(std::memory_order_acquire));
	assertInLoopThread();
	quit_.store(false, std::memory_order_release);
	while (!quit_.load(std::memory_order_acquire))
	{
		active_chs_.clear();
		LOG_TRACE << "wait for tasks";
		TimeStamp time_stamp = epoller_->poll(&active_chs_);
		LOG_TRACE << "tasks is coming";
		for (auto ch_ptr : active_chs_)
		{
			ch_ptr->handleEvent(time_stamp);
		}
		doPendingFuncs();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
}

TimerId EventLoop::runAt(TimeStamp time_stamp, const std::function<void()>& cb)
{
	return tq_->addTimer(time_stamp, cb, -1);
}

TimerId EventLoop::runAfter(double delay, const std::function<void()>& cb)
{
	return tq_->addTimer(TimeStamp::addTime(lynx::TimeStamp::now(), delay), cb,
						 -1);
}

TimerId EventLoop::runEvery(double interval, const std::function<void()>& cb)
{
	return tq_->addTimer(TimeStamp::addTime(lynx::TimeStamp::now(), interval),
						 cb, interval);
}

void EventLoop::cancell(TimerId timer_id)
{
	tq_->cancell(timer_id);
}