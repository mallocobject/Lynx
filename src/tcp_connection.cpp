#include "lynx/include/tcp_connection.h"
#include "lynx/include/buffer.h"
#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <cassert>
#include <cerrno>
#include <cstddef>
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
	  peer_port_(port), state_(Disconnected),
	  input_buffer_(std::make_shared<Buffer>()),
	  output_buffer_(std::make_shared<Buffer>())
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
	assert(loop_->isInLocalThread());
	ch_->tie(weak_from_this());
	ch_->enableIN();
	ch_->useET();
	connect_callback_(shared_from_this());
}

void TcpConnection::destroy()
{
	assert(loop_->isInLocalThread());
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
	assert(loop_->isInLocalThread());
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
	assert(loop_->isInLocalThread());
	int saved_errno = 0;
	ssize_t n = input_buffer_->readFd(fd_, &saved_errno);
	if (n > 0)
	{
		message_callback_(shared_from_this(), input_buffer_);
		// LOG_INFO() << ;
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		LOG_ERROR() << "TcpConnection::handleRead [" << errno
					<< "]: " << strerror(saved_errno);
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	assert(loop_->isInLocalThread());
	if (ch_->IsWriting())
	{
		ssize_t n = ::write(fd_, output_buffer_->peek(),
							output_buffer_->readableBytes());
		if (n > 0)
		{
			output_buffer_->retrieve(n);
			if (output_buffer_->readableBytes() == 0)
			{
				ch_->disableOUT();
				write_complete_callback_(shared_from_this());
			}
		}
	}
}

void TcpConnection::send(const std::string& message)
{
	size_t remaining = message.size();
	size_t n_wrote = 0;

	// 先调用write尝试发送，将剩余的数据存放至output buffer
	if (!ch_->IsWriting() && output_buffer_->readableBytes() == 0)
	{
		n_wrote = ::write(ch_->fd(), message.data(), message.size());
		if (n_wrote >= 0)
		{
			remaining -= n_wrote;
		}
		else
		{
			LOG_ERROR() << "TcpConnection::send [" << errno
						<< "]: " << strerror(errno);
			handleError();
		}
	}

	if (remaining > 0)
	{
		output_buffer_->append(message.data() + n_wrote, remaining);
		if (!ch_->IsWriting())
		{
			ch_->enableOUT();
		}
	}
}
} // namespace lynx