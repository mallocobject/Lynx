#ifndef LYNX_CHANNEL_H
#define LYNX_CHANNEL_H

#include "lynx/include/common.h"
#include <cstdint>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <utility>

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
	int index_; // used by epoller

	std::function<void()> read_callback_;
	std::function<void()> write_callback_;
	std::function<void()> close_callback_;
	std::function<void()> error_callback_;

  public:
	DISABLE_COPY(Channel)

	Channel(int fd, EventLoop* loop);
	~Channel();

	void remove();

	void tie(std::weak_ptr<void> obj)
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

	int index() const
	{
		return index_;
	}

	void setIndex(int idx)
	{
		index_ = idx;
	}

	void setEvents(uint32_t events)
	{
		events_ = events;
	}

	uint32_t events() const
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

	void setReuseAddr(bool on)
	{
		int optval = on ? 1 : 0;
		setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}

	void setKeepAlive(bool on)
	{
		int optval = on ? 1 : 0;
		setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	}

	void setNoDelay(bool on)
	{
		int optval = on ? 1 : 0;
		setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}

	void disableIN();
	void disableOUT();
	bool IsWriting() const;

	void disAll();

	void setReadCallback(std::function<void()> cb)
	{
		read_callback_ = std::move(cb);
	}

	void setWriteCallback(std::function<void()> cb)
	{
		write_callback_ = std::move(cb);
	}

	void setCloseCallback(std::function<void()> cb)
	{
		close_callback_ = std::move(cb);
	}

	void setErrorCallback(std::function<void()> cb)
	{
		error_callback_ = std::move(cb);
	}

	void handleEvent();
	void handleEventWithGuard();
};
} // namespace lynx

#endif