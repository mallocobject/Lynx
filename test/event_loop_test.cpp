// test_thread_id.cpp
#include "lynx/net/event_loop.h"
#include <iostream>
#include <thread>
#include <vector>

void test_thread_id()
{
	std::cout << "Thread " << lynx::CurrentThread::tid()
			  << " (native: " << std::this_thread::get_id() << ")" << std::endl;

	// 验证多次调用返回相同值
	uint64_t tid1 = lynx::CurrentThread::tid();
	uint64_t tid2 = lynx::CurrentThread::tid();

	if (tid1 == tid2)
	{
		std::cout << "tid() is consistent: " << tid1 << std::endl;
	}
	else
	{
		std::cout << "ERROR: tid() not consistent!" << std::endl;
	}
}

int main()
{
	std::cout << "Main thread: " << lynx::CurrentThread::tid() << std::endl;

	std::vector<std::thread> threads;
	for (int i = 0; i < 3; ++i)
	{
		threads.emplace_back(test_thread_id);
	}

	for (auto& t : threads)
	{
		t.join();
	}

	// 验证EventLoop的线程检查
	lynx::EventLoop loop;
	std::cout << "EventLoop created in thread: " << lynx::CurrentThread::tid()
			  << std::endl;

	// 应该在主线程中工作
	if (loop.isInLocalThread())
	{
		std::cout << "EventLoop is in the correct thread" << std::endl;
	}

	return 0;
}