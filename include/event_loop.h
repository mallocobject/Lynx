#ifndef LYNX_EVENT_LOOP_H
#define LYNX_EVENT_LOOP_H

#include "lynx/include/common.h"
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
class Epoller;
class Channel;
class EventLoop
{
  private:
	std::unique_ptr<Epoller> epoller_;
	const std::thread::id tid_;
	std::atomic<bool> looping_;
	std::atomic<bool> quit_;
	std::atomic<bool> calling_pending_functors_;
	int wakeup_fd_; // for eventfd
	std::unique_ptr<Channel> wakeup_ch_;
	std::mutex mtx_;
	std::vector<std::function<void()>> pending_functors_; // guarded by mutex

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
		return std::this_thread::get_id() == tid_;
	}

	void updateChannel(Channel* ch);
	void deleteChannel(Channel* ch);

	void run();

	void quit();

	void exeInLocalThread(const std::function<void()> cb);

	void queueInLocalThread(const std::function<void()> cb);

  private:
	void abortNotInLocalThread();
	void wakeup()
	{
		uint64_t signal = 1;
		ssize_t n = ::write(wakeup_fd_, &signal, sizeof(signal));
		assert(n == sizeof(signal));
	}
	void handleRead() // for wakeup
	{
		uint64_t signal = 1;
		ssize_t n = ::read(wakeup_fd_, &signal, sizeof(signal));
		assert(n == sizeof(signal));
	}
	void doPendingFunctors();
};
} // namespace lynx

#endif