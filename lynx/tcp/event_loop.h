#ifndef LYNX_EVENT_LOOP_H
#define LYNX_EVENT_LOOP_H

#include "lynx/base/current_thread.hpp"
#include "lynx/base/noncopyable.hpp"
#include "lynx/logger/logger.h"
#include "lynx/time/timer_id.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>
namespace lynx
{
class Epoller;
class Channel;
class TimerQueue;
class EventLoop : public noncopyable
{
  private:
	std::unique_ptr<Epoller> epoller_;
	const uint64_t tid_;
	std::atomic<bool> quit_;
	std::atomic<bool> calling_pending_funcs_;

	int wakeup_fd_; // for eventfd
	std::unique_ptr<Channel> wakeup_ch_;
	std::mutex mtx_;
	std::vector<std::function<void()>> pending_funcs_; // guarded by mutex

	std::vector<Channel*> active_chs_;
	std::unique_ptr<TimerQueue> tq_;

  public:
	EventLoop();
	~EventLoop();

	void assertInLoopThread()
	{
		if (!InLoopThread())
		{
			abortNotInLoopThread();
		}
	}

	void updateChannel(Channel* ch);
	void removeChannel(Channel* ch);

	void run();

	void quit()
	{
		{
			quit_.store(true, std::memory_order_release);
			if (!InLoopThread())
			{
				wakeup();
			}
		}
	}

	void runInLoop(const std::function<void()>& cb)
	{
		if (InLoopThread())
		{
			cb();
		}
		else
		{
			queueInLoop(cb);
		}
	}

	void queueInLoop(const std::function<void()>& cb)
	{
		{
			std::lock_guard<std::mutex> lock(mtx_);
			pending_funcs_.push_back(cb);
		}

		// 只用在 IO 线程的事件回调中调用 queueInLocalThread() 才无需 wakeup
		// 因为 doPendingFuncs() 在事件回调之后
		if (!InLoopThread() || calling_pending_funcs_)
		{
			wakeup();
		}
	}

	bool InLoopThread() const
	{
		return CurrentThread::tid() == tid_;
	}

	TimerId runAt(TimeStamp time_stamp, const std::function<void()>& cb);
	TimerId runAfter(double delay, const std::function<void()>& cb);
	TimerId runEvery(double interval, const std::function<void()>& cb);

	void cancell(TimerId timer_id);

  private:
	void abortNotInLoopThread()
	{
		LOG_FATAL << "EventLoop was created in threadId_ = " << tid_
				  << ", current thread id = " << std::this_thread::get_id();
	}

	void doPendingFuncs()
	{
		std::vector<std::function<void()>> functors;
		calling_pending_funcs_ = true;
		// 缩小临界区
		// 避免回调地狱
		{
			std::lock_guard<std::mutex> lock(mtx_);
			std::swap(functors, pending_funcs_);
		}
		for (auto& f : functors)
		{
			f();
		}
		calling_pending_funcs_ = false;
	}

	void wakeup()
	{
		uint64_t signal = 1;
		ssize_t n = ::write(wakeup_fd_, &signal, sizeof(signal));

		if (n != sizeof(signal))
		{
			LOG_ERROR << "EventLoop::wakeup() writes " << n
					  << " bytes instead of 8";
		}
	}

	void handleRead() // for wakeup
	{
		uint64_t signal = 1;
		ssize_t n = ::read(wakeup_fd_, &signal, sizeof(signal));

		if (n != sizeof(signal))
		{
			LOG_ERROR << "EventLoop::handleRead() reads " << n
					  << " bytes instead of 8";
		}
	}
};
} // namespace lynx

#endif