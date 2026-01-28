#include "lynx/include/acceptor.h"
#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

namespace lynx
{
Acceptor::Acceptor(EventLoop* loop, const char* ip, uint16_t port)
	: loop_(loop), listening_(false),
	  idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd_ < 0)
	{
		LOG_FATAL << "Acceptor::Acceptor [" << errno
				  << "]: " << strerror(errno);
		exit(1);
	}
	ch_ = std::make_unique<Channel>(fd_, loop);
	ch_->setReuseAddr(true);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	::inet_pton(AF_INET, ip, &addr.sin_addr);
	ch_->bind((sockaddr*)&addr);

	ch_->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
	::close(idle_fd_);
}

void Acceptor::listen()
{
	loop_->assertInLocalThread();
	listening_ = true;
	ch_->listen();
	ch_->enableIN();
}

void Acceptor::handleRead()
{
	loop_->assertInLocalThread();

	sockaddr_in peer_addr;
	::bzero(&peer_addr, sizeof(sockaddr_in));
	int saved_errno = 0;
	int conn_fd = ch_->accept((sockaddr*)&peer_addr, &saved_errno);
	if (conn_fd >= 0)
	{
		if (new_connection_callback_)
		{
			char peer_ip[INET_ADDRSTRLEN];
			bzero(peer_ip, sizeof(peer_ip));
			::inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, INET_ADDRSTRLEN);
			uint16_t peer_port = ::ntohs(peer_addr.sin_port);

			new_connection_callback_(conn_fd, peer_ip, peer_port);
		}
		else
		{
			::close(conn_fd);
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
			conn_fd = ::accept(fd_, nullptr, nullptr);
			::close(conn_fd);
			idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			LOG_ERROR << "EMFILE: Out of file descriptors!";
			break;
		default:
			LOG_FATAL << "Epoller::wait [" << saved_errno
					  << "]: " << strerror(saved_errno);
			break;
		}
	}
}
} // namespace lynx