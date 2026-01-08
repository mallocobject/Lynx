#ifndef LYNX_CHANNEL_H
#define LYNX_CHANNEL_H

#include "lynx/include/common.h"
#include <cstdint>
#include <fcntl.h>
#include <memory>

namespace lynx
{
class EventLoop;
class Channel
{
  private:
	int fd_;
	EventLoop* loop_;
	uint32_t events_;  // interesting events
	uint32_t revents_; // ready events
	std::weak_ptr<void> tie_;
	bool tied_;
	bool in_epoll_;

  public:
	DISABLE_DEFAULT(Channel)
	DISABLE_COPY_AND_MOVE(Channel)

	Channel(int fd, EventLoop* loop);
	~Channel();

	void tie(std::shared_ptr<void> obj)
	{
		tie_ = obj;
		tied_ = true;
	}

	int fd() const
	{
		return fd_;
	}

	void setInEpoll(bool in_epoll)
	{
		in_epoll_ = in_epoll;
	}

	bool inEpoll() const
	{
		return in_epoll_;
	}

	EventLoop* loop() const
	{
		return loop_;
	}

	void setEvents(uint32_t events)
	{
		events_ = events;
	}

	uint32_t getEvents() const
	{
		return events_;
	}

	void setRevents(uint32_t revents)
	{
		revents_ = revents;
	}

	// uint32_t getRevents() const
	// {
	// 	return revents_;
	// }

	void useET();
	void enableIN();
	void enableOUT();
	void enableRDHUP();

	void setNonBlocking()
	{
		int flags = fcntl(fd_, F_GETFL, 0);
		fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
	}

	void disableIN();
	void disableOUT();

	void handle();
	void handleWithGuard();
};
} // namespace lynx

#endif