#include "lynx/include/tcp_connection.h"
#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <cassert>
#include <cstring>
#include <memory>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace lynx
{
TcpConnection::TcpConnection(int fd, EventLoop* loop, const char* ip,
							 uint16_t port)
	: ch_(std::make_unique<Channel>(fd, loop)), fd_(fd), loop_(loop),
	  peer_port_(port), state_(Disconnected)
{
	strcpy(peer_ip_, ip);
	ch_->setKeepAlive(true);

	ch_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	ch_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	ch_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	ch_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() = default;

void TcpConnection::setTcpReuseAddr(bool on)
{
	ch_->setReuseAddr(on);
}

void TcpConnection::setTcpNoDelay(bool on)
{
	ch_->setNoDelay(on);
}

void TcpConnection::establish()
{
	assert(loop_->isLocalThread());
	// try
	// {
	// 	auto self = shared_from_this();
	// 	LOG_DEBUG() << "shared_from_this() successful, use_count: "
	// 				<< self.use_count();
	// }
	// catch (const std::bad_weak_ptr& e)
	// {
	// 	LOG_ERROR() << "bad_weak_ptr: " << e.what();
	// 	LOG_ERROR() << "This TcpConnection is not managed by shared_ptr!";
	// }
	ch_->tie(weak_from_this());
	ch_->enableIN();
	ch_->useET();
	connect_callback_(shared_from_this());
}

void TcpConnection::destroy()
{
	assert(loop_->isLocalThread());
	ch_->remove();
}

void TcpConnection::handleError()
{
	int error = 0;
	socklen_t len = sizeof(error);
	int err = getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len);
	LOG_ERROR() << "TcpConnection::handleError [" << err
				<< "]: " << strerror(err);
}

void TcpConnection::handleClose()
{
	assert(loop_->isLocalThread());
	LOG_INFO() << "Connection closed - FD: " << fd_ << ", Peer: " << peer_ip_
			   << ":" << peer_port_;
	// assert(state_ == Connected || state_ == Disconnecting);
	state_ = Disconnected;
	ch_->disAll();
	connect_callback_(shared_from_this());
	close_callback_(shared_from_this());
}

void TcpConnection::handleRead()
{
	char buf[1024];
	bzero(buf, sizeof(buf));
	ssize_t n = read(fd_, buf, sizeof(buf) - 1);
	if (n > 0)
	{
		message_callback_(shared_from_this());
		buf[n] = 0;
		LOG_INFO() << buf;
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		LOG_ERROR() << "TcpConnection::handleRead [" << errno
					<< "]: " << strerror(errno);
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	assert(loop_->isLocalThread());
}
} // namespace lynx