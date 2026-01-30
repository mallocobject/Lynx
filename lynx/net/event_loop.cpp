#include "lynx/net/event_loop.h"
#include "lynx/base/logger.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/base/timer_queue.h"
#include "lynx/net/channel.h"
#include "lynx/net/epoller.h"
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <thread>
#include <utility>
#include <vector>

namespace lynx
{
EventLoop::EventLoop()
	: epoller_(std::make_unique<Epoller>()), tid_(std::this_thread::get_id()),
	  looping_(false), quit_(true), calling_pending_functors_(false),
	  wakeup_fd_(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
	  wakeup_ch_(std::make_unique<Channel>(wakeup_fd_, this)),
	  timer_queue_(std::make_unique<TimerQueue>(this))
{
	wakeup_ch_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	wakeup_ch_->enableIN();
	LOG_DEBUG << "EventLoop created " << this << " in thread " << tid_;
}

EventLoop::~EventLoop()
{
}

void EventLoop::updateChannel(Channel* ch)
{
	assert(ch->loop() == this);
	assertInLocalThread();
	epoller_->updateChannel(ch);
}

void EventLoop::deleteChannel(Channel* ch)
{
	assert(ch->loop() == this);
	assertInLocalThread();
	epoller_->deleteChannel(ch);
}

void EventLoop::run()
{
	assert(!looping_);
	assertInLocalThread();
	looping_ = true;
	quit_ = false;
	while (!quit_)
	{
		active_chs_.clear();
		TimeStamp time_stamp = epoller_->wait(&active_chs_);
		for (auto ch_ptr : active_chs_)
		{
			ch_ptr->handleEvent(time_stamp);
		}
		doPendingFunctors();
	}
	LOG_DEBUG << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLocalThread())
	{
		wakeup();
	}
}

void EventLoop::runInLocalThread(const std::function<void()> cb)
{
	if (isInLocalThread())
	{
		cb();
	}
	else
	{
		queueInLocalThread(cb);
	}
}

void EventLoop::queueInLocalThread(const std::function<void()> cb)
{
	{
		std::lock_guard<std::mutex> lock(mtx_);
		pending_functors_.push_back(cb);
	}

	// 只用在 IO 线程的事件回调中调用 queueInLocalThread() 才无需 wakeup
	// 因为 doPendingFunctors() 在事件回调之后
	if (!isInLocalThread() || calling_pending_functors_)
	{
		wakeup();
	}
}

void EventLoop::abortNotInLocalThread()
{
	if (!isInLocalThread())
	{
		LOG_FATAL << "EventLoop was created in threadId_ = " << tid_
				  << ", current thread id = " << std::this_thread::get_id();
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<std::function<void()>> functors;
	calling_pending_functors_ = true;
	// 缩小临界区
	// 减轻阻塞调用 doPendingFunctors
	// 避免死锁
	{
		std::lock_guard<std::mutex> lock(mtx_);
		std::swap(functors, pending_functors_);
	}
	for (auto& f : functors)
	{
		f();
	}
	calling_pending_functors_ = false;
}

void EventLoop::runAt(TimeStamp time_stamp, const std::function<void()>& cb)
{
	timer_queue_->addTimer(time_stamp, cb, -1);
}

void EventLoop::runAfter(double delay, const std::function<void()>& cb)
{
	timer_queue_->addTimer(TimeStamp::addTime(lynx::TimeStamp::now(), delay),
						   cb, -1);
}

void EventLoop::runEvery(double interval, const std::function<void()>& cb)
{
	timer_queue_->addTimer(TimeStamp::addTime(lynx::TimeStamp::now(), interval),
						   cb, interval);
}
} // namespace lynx