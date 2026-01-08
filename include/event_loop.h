#ifndef LYNX_EVENT_LOOP_H
#define LYNX_EVENT_LOOP_H

#include "lynx/include/common.h"
#include <memory>
#include <thread>
namespace lynx
{
class Epoller;
class Channel;
class EventLoop
{
  private:
	std::unique_ptr<Epoller> epoller_;
	std::thread::id tid_;

  public:
	DISABLE_COPY_AND_MOVE(EventLoop)

	EventLoop();
	~EventLoop();

	bool isLocalThread() const
	{
		return std::this_thread::get_id() == tid_;
	}

	void updateChannel(Channel* ch);
	void deleteChannel(Channel* ch);

	void run();
};
} // namespace lynx

#endif