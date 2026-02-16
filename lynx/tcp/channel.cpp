#include "lynx/tcp/channel.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/event_loop.hpp"
#include "lynx/tcp/socket.hpp"
#include <cassert>
using namespace lynx;

Channel::Channel(int fd, EventLoop* loop)
	: fd_(fd), loop_(loop), events_(0), revents_(0), in_epoll_(false),
	  epoll_index_(-1), tied_(false)
{
}

Channel::~Channel()
{
	if (fd_ != -1)
	{
		Socket::close(fd_);
		fd_ = -1;
	}
}

void Channel::remove()
{
	assert(loop_);
	loop_->removeChannel(this); // sync
}

void Channel::update()
{
	LOG_TRACE << "fd: " << fd_ << " starts update state";
	assert(loop_);
	loop_->updateChannel(this); // sync
	LOG_TRACE << "fd: " << fd_ << " has updated state";
}

void Channel::handleEventWithGuard(TimeStamp time_stamp)
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