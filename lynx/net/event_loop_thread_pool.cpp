#include "lynx/net/event_loop_thread_pool.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/event_loop_thread.h"
#include <memory>

namespace lynx
{
EventLoopThreadPool::EventLoopThreadPool(EventLoop* main_loop, size_t num_ths)
	: main_loop_(main_loop), num_ths_(num_ths), next_loop_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::startup()
{
	for (int i = 0; i < num_ths_; i++)
	{
		vec_loop_th.push_back(std::make_unique<EventLoopThread>());
		vec_loop_.push_back(vec_loop_th.back()->startup());
	}
}
} // namespace lynx