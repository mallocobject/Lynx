#ifndef LYNX_EVENT_LOOP_THREAD_H
#define LYNX_EVENT_LOOP_THREAD_H

#include "lynx/include/common.h"
#include <condition_variable>
#include <mutex>
#include <thread>
namespace lynx
{
class EventLoop;

// one loop per thread
class EventLoopThread
{
  private:
	std::mutex mtx_;
	std::condition_variable cv_;
	EventLoop* loop_;
	std::thread th_;

  public:
	DISABLE_COPY(EventLoopThread)

	EventLoopThread();
	~EventLoopThread();

	EventLoop* startup();

  private:
	void ThreadFunc();
};
} // namespace lynx

#endif