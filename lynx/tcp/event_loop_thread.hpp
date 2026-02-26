#ifndef LYNX_TCP_EVENT_LOOP_THREAD_HPP
#define LYNX_TCP_EVENT_LOOP_THREAD_HPP

#include "lynx/base/noncopyable.hpp"
#include <latch>
#include <thread>
namespace lynx
{
namespace tcp
{
class EventLoop;
class EventLoopThread : public base::noncopyable
{
  private:
	EventLoop* loop_;
	std::thread thread_;
	std::latch latch_down_;

  public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop* run();

  private:
	void threadWorker();
};
} // namespace tcp
} // namespace lynx

#endif