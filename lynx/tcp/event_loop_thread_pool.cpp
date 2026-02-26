#include "lynx/tcp/event_loop_thread_pool.hpp"
#include "lynx/tcp/event_loop_thread.hpp"

using namespace lynx;
using namespace lynx::tcp;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* main_loop,
										 size_t thread_num)
	: main_loop_(main_loop), thread_num_(thread_num), next_loop_index_(0)

{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::run()
{
	for (int i = 0; i < thread_num_; i++)
	{
		loop_thread_pool_.push_back(std::make_unique<EventLoopThread>());
		sub_loops_.push_back(loop_thread_pool_.back()->run());
	}
}