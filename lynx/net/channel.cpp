#include "lynx/net/channel.h"
#include "lynx/base/logger.hpp"
#include "lynx/net/event_loop.h"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
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
	if (fd_ != -1)
	{
		::close(fd_);
		fd_ = -1;
	}
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

void Channel::bind(sockaddr* addr)
{
	int ret = ::bind(fd_, addr, sizeof(sockaddr));
	if (ret < 0)
	{
		LOG_FATAL << "Channel::bind [" << errno << "]: " << strerror(errno);
		exit(1);
	}
}

void Channel::listen()
{
	::listen(fd_, SOMAXCONN);
}

int Channel::accept(sockaddr* peer_addr, int* saved_errno)
{
	socklen_t addr_len = sizeof(sockaddr);
	int conn_fd =
		::accept4(fd_, peer_addr, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (conn_fd < 0)
	{
		*saved_errno = errno;
		LOG_WARN << "Channel::accept [" << errno << "]: " << strerror(errno);
	}

	return conn_fd;
}

void Channel::connect(sockaddr* serv_addr, int* saved_errno)
{
	*saved_errno = 0;

	int ret = ::connect(fd_, serv_addr, sizeof(sockaddr));
	if (ret < 0)
	{
		*saved_errno = errno;
		LOG_WARN << "Channel::connect [" << errno << "]: " << strerror(errno);
	}
}

int Channel::getSocketError() const
{
	int error = 0;
	socklen_t len = sizeof(error);
	if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
	{
		error = errno;
	}
	return error;
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

void Channel::disableAll()
{
	assert(loop_ != nullptr);
	events_ = 0;
	loop_->updateChannel(this);
}

bool Channel::writing() const
{
	return events_ & EPOLLOUT;
}

void Channel::handleEvent(TimeStamp time_stamp)
{
	if (tied_)
	{
		tie_.lock();
		handleEventWithGuard(time_stamp);
	}
	else
	{
		handleEventWithGuard(time_stamp);
	}
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
} // namespace lynx