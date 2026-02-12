#ifndef LYNX_CHANNEL_H
#define LYNX_CHANNEL_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/logger/logger.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <sys/epoll.h>
namespace lynx
{
class EventLoop;
class Channel : public noncopyable
{
  private:
	int fd_;
	EventLoop* loop_;
	uint32_t events_;
	uint32_t revents_;
	std::weak_ptr<void> tie_with_conn_;
	bool tied_;
	bool in_epoll_;
	int epoll_index_;

	std::function<void()> read_callback_;
	std::function<void()> write_callback_;
	std::function<void()> close_callback_;
	std::function<void()> error_callback_;

  public:
	Channel(int fd, EventLoop* loop);
	~Channel();

	void remove();

	void tie(std::weak_ptr<void> obj)
	{
		tie_with_conn_ = obj;
		tied_ = true;
	}

	int fd() const
	{
		return fd_;
	}

	int releaseFd()
	{
		int tmp = fd_;
		fd_ = -1;
		return tmp;
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

	int epollIndex() const
	{
		return epoll_index_;
	}

	void setEpollIndex(int idx)
	{
		epoll_index_ = idx;
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

	void useET()
	{
		assert(loop_ != nullptr);
		events_ |= EPOLLET;
		update();
	};

	void enableIN()
	{
		assert(loop_ != nullptr);
		events_ |= EPOLLIN;
		update();
	}

	void enableOUT()
	{
		assert(loop_ != nullptr);
		events_ |= EPOLLOUT;
		update();
	}

	void enableRDHUP()
	{
		assert(loop_ != nullptr);
		events_ |= EPOLLRDHUP;
		update();
	}

	void disableIN()
	{
		assert(loop_ != nullptr);
		events_ &= ~EPOLLIN;
		update();
	}

	void disableOUT()
	{
		assert(loop_ != nullptr);
		events_ &= ~EPOLLOUT;
		update();
	}

	void disableAll()
	{
		assert(loop_ != nullptr);
		events_ = 0;
		update();
	}

	bool writing() const
	{
		return events_ & EPOLLOUT;
	}

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

	void handleEvent(TimeStamp time_stamp)
	{
		if (tied_)
		{
			auto guard = tie_with_conn_.lock();
			if (guard == nullptr)
			{
				LOG_FATAL << "weak_ptr -> shared_ptr error";
				exit(EXIT_FAILURE);
			}
			handleEventWithGuard(time_stamp);
		}
		else
		{
			handleEventWithGuard(time_stamp);
		}
	}

  private:
	void update();
	void handleEventWithGuard(TimeStamp time_stamp);
};
} // namespace lynx

#endif