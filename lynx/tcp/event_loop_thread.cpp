#include "lynx/tcp/event_loop_thread.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/event_loop.hpp"
#include <functional>
#include <latch>
#include <thread>

using namespace lynx;
using namespace lynx::tcp;

EventLoopThread::EventLoopThread() : loop_(nullptr), latch_down_(1)
{
}

EventLoopThread::~EventLoopThread()
{
	if (thread_.joinable())
	{
		thread_.join();
	}
}

EventLoop* EventLoopThread::run()
{
	LOG_TRACE << "EventLoop starts initializing";
	thread_ = std::thread(std::bind(&EventLoopThread::threadWorker, this));
	EventLoop* loop = nullptr;
	latch_down_.wait();
	LOG_TRACE << "EventLoop has initialized";
	loop = loop_;
	return loop;
}

void EventLoopThread::threadWorker()
{
	EventLoop loop;
	loop_ = &loop;
	latch_down_.count_down();
	loop_->run();

	loop_ = nullptr;
}