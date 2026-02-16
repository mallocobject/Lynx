#include "lynx/tcp/acceptor.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/channel.hpp"
#include "lynx/tcp/event_loop.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include "lynx/tcp/socket.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <unistd.h>

using namespace lynx;

Acceptor::Acceptor(EventLoop* loop, const InetAddr& local_addr)
	: loop_(loop), addr_(local_addr), listening_(false), idle_fd_(-1)
{
	idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
	if (idle_fd_ == -1)
	{
		LOG_WARN << "open /dev/null failed: " << ::strerror(errno);
	}

	int saved_errno = 0;
	int fd = Socket::socket(&saved_errno);
	checkErrno(saved_errno);

	Socket::setReuseAddr(fd);
	Socket::bind(fd, local_addr, &saved_errno);
	checkErrno(saved_errno);

	ch_ = std::make_unique<Channel>(fd, loop);
	ch_->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
	if (idle_fd_ != -1)
	{
		::close(idle_fd_);
		idle_fd_ = -1;
	}
}

void Acceptor::listen()
{
	LOG_TRACE << "acceptor starts listening";

	loop_->assertInLoopThread();

	listening_ = true;
	int saved_errno = 0;

	Socket::listen(ch_->fd(), &saved_errno);
	checkErrno(saved_errno);

	ch_->enableIN();
}

int Acceptor::fd() const
{
	return ch_->fd();
}

void Acceptor::handleRead()
{
	loop_->assertInLoopThread();

	InetAddr peer_addr;
	int saved_errno = 0;

	int conn_fd = Socket::accept(ch_->fd(), &peer_addr, &saved_errno);

	if (conn_fd >= 0)
	{
		if (new_connection_callback_)
		{
			new_connection_callback_(conn_fd, peer_addr);
		}
		else
		{
			Socket::close(conn_fd);
			conn_fd = -1;
		}
	}
	else
	{
		switch (saved_errno)
		{
		case EAGAIN:
		case EINTR:
		case ECONNABORTED:
			break;
		case EMFILE:
			::close(idle_fd_);
			conn_fd = Socket::accept(ch_->fd(), &peer_addr, nullptr);
			Socket::close(conn_fd);
			idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			LOG_ERROR << "EMFILE: Out of file descriptors!";
			break;
		default:
			LOG_FATAL << strerror(saved_errno);
			checkErrno(saved_errno);
			break;
		}
	}
}