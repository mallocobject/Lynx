#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <cassert>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace lynx
{
Channel::Channel(int fd, EventLoop* loop)
	: fd_(fd), loop_(loop), events_(0), revents_(0), tied_(false),
	  in_epoll_(false), index_(-1)
{
}

Channel::~Channel()
{

	if (loop_ && in_epoll_)
	{
		loop_->deleteChannel(this);
	}
	close(fd_);
}

void Channel::remove()
{
	if (loop_)
	{
		loop_->deleteChannel(this);
	}
}

void Channel::useET()
{
	assert(loop_ != nullptr);
	events_ |= EPOLLET;
	loop_->updateChannel(this);
}

void Channel::enableIN()
{
	assert(loop_ != nullptr);
	events_ |= EPOLLIN;
	loop_->updateChannel(this);
}

void Channel::enableOUT()
{
	assert(loop_ != nullptr);
	events_ |= EPOLLOUT;
	loop_->updateChannel(this);
}

void Channel::enableRDHUP()
{
	assert(loop_ != nullptr);
	events_ |= EPOLLRDHUP;
	loop_->updateChannel(this);
}

void Channel::disableIN()
{
	assert(loop_ != nullptr);
	events_ &= ~EPOLLIN;
	loop_->updateChannel(this);
}

void Channel::disableOUT()
{
	assert(loop_ != nullptr);
	events_ &= ~EPOLLOUT;
	loop_->updateChannel(this);
}

void Channel::disAll()
{
	assert(loop_ != nullptr);
	events_ = 0;
	loop_->updateChannel(this);
}

bool Channel::IsWriting() const
{
	return events_ & EPOLLOUT;
}

void Channel::handleEvent()
{
	if (tied_)
	{
		tie_.lock();
		handleEventWithGuard();
	}
	else
	{
		handleEventWithGuard();
	}
}

void Channel::handleEventWithGuard()
{
	// 如果有 HUP 但没有 IN，说明连接彻底断开且没有残留数据，直接触发关闭
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
	{
		if (close_callback_)
		{
			close_callback_();
		}
	}
	if (revents_ & EPOLLERR)
	{
		if (error_callback_)
		{
			error_callback_();
		}
	}
	// EPOLLPRI：紧急数据/带外数据
	// EPOLLRDHUP：对端关闭连接（半关闭），依然要读，因为可能还有残留数据在缓冲区
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (read_callback_)
		{
			read_callback_();
		}
	}
	if (revents_ & EPOLLOUT)
	{
		if (write_callback_)
		{
			write_callback_();
		}
	}
}
} // namespace lynx