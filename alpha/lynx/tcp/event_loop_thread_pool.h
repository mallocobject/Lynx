#ifndef LYNX_EVENT_LOOP_THREAD_POOL_H
#define LYNX_EVENT_LOOP_THREAD_POOL_H

#include "lynx/base/noncopyable.hpp"
#include <cstddef>
#include <memory>
#include <vector>
namespace lynx
{
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool : public noncopyable
{
  private:
	size_t thread_num_;
	EventLoop* main_loop_;
	std::vector<EventLoop*> sub_loops_;
	std::vector<std::unique_ptr<EventLoopThread>> loop_thread_pool_;
	size_t next_loop_index_;

  public:
	EventLoopThreadPool(EventLoop* main_loop, size_t thread_num);
	~EventLoopThreadPool();

	void run();

	EventLoop* nextLoop()
	{
		EventLoop* loop = main_loop_;

		if (!sub_loops_.empty())
		{
			loop = sub_loops_[next_loop_index_++];
			if (next_loop_index_ >= sub_loops_.size())
			{
				next_loop_index_ = 0;
			}
		}
		return loop;
	}
};
} // namespace lynx

#endif