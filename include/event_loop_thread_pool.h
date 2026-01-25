#ifndef LYNX_EVENT_LOOP_THREAD_POOL_H
#define LYNX_EVENT_LOOP_THREAD_POOL_H

#include "lynx/include/common.h"
#include <cstddef>
#include <memory>
#include <vector>
namespace lynx
{
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool
{
  private:
	size_t num_ths_;
	EventLoop* main_loop_;
	std::vector<EventLoop*> vec_loop_;
	std::vector<std::unique_ptr<EventLoopThread>> vec_loop_th;
	size_t next_loop_;

  public:
	DISABLE_COPY(EventLoopThreadPool)

	EventLoopThreadPool(EventLoop* main_loop, size_t num_ths);
	~EventLoopThreadPool();

	void startup();

	EventLoop* getNextLoop()
	{
		EventLoop* loop = main_loop_;
		if (!vec_loop_.empty())
		{
			loop = vec_loop_[next_loop_++];
			if (next_loop_ >= vec_loop_.size())
			{
				next_loop_ = 0;
			}
		}
		return loop;
	}
};
} // namespace lynx

#endif