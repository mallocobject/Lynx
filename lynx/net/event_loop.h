#ifndef LYNX_EVENT_LOOP_H
#define LYNX_EVENT_LOOP_H

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/logger/logger.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
namespace lynx
{
namespace CurrentThread
{
inline uint64_t tid()
{
	static thread_local uint64_t cached_tid =
		std::hash<std::thread::id>{}(std::this_thread::get_id());
	return cached_tid;
}
} // namespace CurrentThread

class Epoller;
class Channel;
class TimerQueue;
class EventLoop
{
  private:
	std::unique_ptr<Epoller> epoller_;
	const uint64_t tid_;
	std::atomic<bool> looping_;
	std::atomic<bool> quit_;
	std::atomic<bool> calling_pending_functors_;
	int wakeup_fd_; // for eventfd
	std::unique_ptr<Channel> wakeup_ch_;
	std::mutex mtx_;
	std::vector<std::function<void()>> pending_functors_; // guarded by mutex

	std::vector<Channel*> active_chs_;
	std::unique_ptr<TimerQueue> timer_queue_;

  public:
	DISABLE_COPY(EventLoop)

	EventLoop();
	~EventLoop();

	void assertInLocalThread()
	{
		if (!isInLocalThread())
		{
			abortNotInLocalThread();
		}
	}

	bool isInLocalThread() const
	{
		return CurrentThread::tid() == tid_;
	}

	void updateChannel(Channel* ch);
	void deleteChannel(Channel* ch);

	void run();

	void quit();

	void runInLocalThread(const std::function<void()> cb);
	void queueInLocalThread(const std::function<void()> cb);

	void runAt(TimeStamp time_stamp, const std::function<void()>& cb);
	void runAfter(double delay, const std::function<void()>& cb);
	void runEvery(double interval, const std::function<void()>& cb);

  private:
	void abortNotInLocalThread();
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
	void doPendingFunctors();
};
} // namespace lynx

#endif