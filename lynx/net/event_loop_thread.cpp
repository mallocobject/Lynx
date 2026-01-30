#include "lynx/net/event_loop_thread.h"
#include "lynx/net/event_loop.h"
#include <functional>
#include <mutex>
#include <thread>

namespace lynx
{
EventLoopThread::EventLoopThread() : loop_(nullptr)
{
}
EventLoopThread::~EventLoopThread()
{
	if (th_.joinable())
	{
		th_.join();
	}
}

EventLoop* EventLoopThread::startup()
{
	th_ = std::thread(std::bind(&EventLoopThread::ThreadFunc, this));
	EventLoop* loop = nullptr;
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock, [this]() { return loop_ != nullptr; });
		loop = loop_;
	}

	return loop;
}

void EventLoopThread::ThreadFunc()
{
	EventLoop loop;
	{
		std::lock_guard<std::mutex> lock(mtx_);
		loop_ = &loop;
		cv_.notify_one();
	}
	loop_->run();

	// close
	{
		std::lock_guard<std::mutex> lock(mtx_);
		loop_ = nullptr;
	}
}
} // namespace lynx