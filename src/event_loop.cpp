#include "lynx/include/event_loop.h"
#include "lynx/include/channel.h"
#include "lynx/include/epoller.h"
#include <memory>
#include <thread>
#include <vector>

namespace lynx
{
EventLoop::EventLoop()
	: epoller_(std::make_unique<Epoller>()), tid_(std::this_thread::get_id())
{
}

EventLoop::~EventLoop() = default;

void EventLoop::updateChannel(Channel* ch)
{
	epoller_->updateChannel(ch);
}

void EventLoop::deleteChannel(Channel* ch)
{
	epoller_->deleteChannel(ch);
}

void EventLoop::run()
{
	while (true)
	{
		std::vector<Channel*> active_chs = epoller_->wait();
		for (auto ch_ptr : active_chs)
		{
			ch_ptr->handle();
		}
	}
}
} // namespace lynx