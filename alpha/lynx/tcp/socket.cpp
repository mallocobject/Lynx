#include "lynx/tcp/socket.h"
#include "lynx/logger/logger.h"
#include <asm-generic/socket.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace lynx;

int Socket::socketErrno(int fd)
{
	int error = 0;
	socklen_t len = sizeof(error);
	if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
	{
		LOG_ERROR << "getsockopt(SO_ERROR) failed for fd " << fd << ": "
				  << ::strerror(errno);
		return -1;
	}
	return error;
}

void Socket::setNonBlocking(int fd, bool on)
{
	int flags = ::fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		LOG_ERROR << "fcntl(F_GETFL) failed for fd " << fd << ": "
				  << ::strerror(errno);
		return;
	}

	if (on)
	{
		flags |= O_NONBLOCK;
	}
	else
	{
		flags &= ~O_NONBLOCK;
	}

	if (::fcntl(fd, F_SETFL, flags) == -1)
	{
		LOG_ERROR << "fcntl(F_SETFL) failed for fd " << fd << ": "
				  << ::strerror(errno);
	}
}

void Socket::setReuseAddr(int fd, bool on)
{
	int optval = on ? 1 : 0;
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==
		-1)
	{
		LOG_ERROR << "setsockopt(SO_REUSEADDR) failed for fd " << fd << ": "
				  << ::strerror(errno);
	}
}
void Socket::setKeepAlive(int fd, bool on)
{
	int optval = on ? 1 : 0;
	if (::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) ==
		-1)
	{
		LOG_ERROR << "setsockopt(SO_KEEPALIVE) failed for fd " << fd << ": "
				  << ::strerror(errno);
	}
}

void Socket::setNoDelay(int fd, bool on)
{
	int optval = on ? 1 : 0;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)))
	{
		LOG_ERROR << "setsockopt(TCP_NODELAY) failed for fd " << fd << ": "
				  << ::strerror(errno);
	}
}

bool Socket::bind(int fd, const InetAddr& local_addr, int* saved_errno)
{
	if (saved_errno)
	{
		*saved_errno = 0;
	}

	if (::bind(fd, local_addr.sockaddr(), sizeof(struct sockaddr_in)) == -1)
	{
		if (saved_errno)
		{
			*saved_errno = errno;
		}
		LOG_ERROR << "bind failed for fd " << fd << ": " << ::strerror(errno);
		return false;
	}
	return true;
}

bool Socket::listen(int fd, int* saved_errno, int backlog)
{
	if (saved_errno)
	{
		*saved_errno = 0;
	}

	if (::listen(fd, backlog) == -1)
	{
		if (saved_errno)
		{
			*saved_errno = errno;
		}
		LOG_ERROR << "listen failed for fd " << fd << ": " << ::strerror(errno);
		return false;
	}
	return true;
}

int Socket::accept(int fd, InetAddr* peer_addr, int* saved_errno)
{
	if (saved_errno)
	{
		*saved_errno = 0;
	}
	socklen_t addr_len = sizeof(sockaddr);
	int conn_fd = ::accept4(fd, peer_addr ? peer_addr->sockaddr() : nullptr,
							&addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (conn_fd == -1)
	{
		if (saved_errno)
		{
			*saved_errno = errno;
		}
	}

	return conn_fd;
}

// for client
bool Socket::connect(int fd, const InetAddr& serv_addr, int* saved_errno)
{
	if (saved_errno)
	{
		*saved_errno = 0;
	}

	if (::connect(fd, serv_addr.sockaddr(), sizeof(sockaddr)) == -1)
	{
		if (saved_errno)
		{
			*saved_errno = socketErrno(fd);
		}
		return false;
	}

	return true;
}

void Socket::shutdown(int fd)
{
	if (::shutdown(fd, SHUT_WR) == -1)
	{
		LOG_ERROR << "shutdown failed for fd " << fd << ": "
				  << ::strerror(errno);
	}
}

void Socket::close(int fd)
{
	if (::close(fd) == -1)
	{
		LOG_ERROR << "close failed for fd " << fd << ": " << ::strerror(errno);
	}
}