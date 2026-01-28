#include "lynx/include/tcp_connection.h"
#include "lynx/include/buffer.h"
#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace lynx
{
TcpConnection::TcpConnection(int fd, EventLoop* loop, const std::string& name,
							 const char* local_ip, uint16_t local_port,
							 const char* peer_ip, uint16_t peer_port)
	: ch_(std::make_unique<Channel>(fd, loop)), fd_(fd), loop_(loop),
	  name_(name), local_port_(local_port), peer_port_(peer_port),
	  state_(Connecting), input_buffer_(std::make_shared<Buffer>()),
	  output_buffer_(std::make_shared<Buffer>()),
	  high_water_mark_(64 * 1024 * 1024), // default 64MB
	  reading_(false)
{
	strcpy(local_ip_, local_ip);
	strcpy(peer_ip_, peer_ip);

	ch_->setKeepAlive(true);

	ch_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	ch_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	ch_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	ch_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::setTcpReuseAddr(bool on)
{
	ch_->setReuseAddr(on);
}

void TcpConnection::setTcpKeepAlive(bool on)
{
	ch_->setKeepAlive(on);
}

void TcpConnection::setTcpNoDelay(bool on)
{
	ch_->setNoDelay(on);
}

void TcpConnection::handleError()
{
	int error = ch_->getSocketError();
	LOG_ERROR << "TcpConnection::handleError [" << name_
			  << "] = SO_ERROR = " << error << ": " << strerror(error);
}

void TcpConnection::handleClose()
{
	loop_->assertInLocalThread();
	if (state_ == Disconnected)
	{
		return;
	}
	LOG_INFO << "Connection closed - FD: " << fd_ << ", Peer: " << peer_ip_
			 << ":" << peer_port_;
	assert(state_ == Connected || state_ == Disconnecting);
	state_ = Disconnected;
	ch_->disableAll();
	if (connect_callback_)
	{
		connect_callback_(shared_from_this());
	}
	if (close_callback_)
	{
		close_callback_(shared_from_this());
	}
}

void TcpConnection::handleRead()
{
	loop_->assertInLocalThread();
	int saved_errno = 0;
	ssize_t n = input_buffer_->readFd(fd_, &saved_errno);
	if (n > 0)
	{
		message_callback_(shared_from_this(), input_buffer_);
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		errno = saved_errno;
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	loop_->assertInLocalThread();
	if (ch_->writing())
	{
		ssize_t n = ::write(fd_, output_buffer_->peek(),
							output_buffer_->readableBytes());
		if (n > 0)
		{
			output_buffer_->retrieve(n);
			if (output_buffer_->readableBytes() == 0)
			{
				ch_->disableOUT();
				if (write_complete_callback_)
				{
					loop_->runInLocalThread(std::bind(write_complete_callback_,
													  shared_from_this()));
				}

				if (state_ == Disconnecting)
				{
					shutdownInLocalLoop();
				}
			}
		}
	}
}

void TcpConnection::send(const std::string& message)
{
	if (state_ == Connected)
	{
		// 手动判断，尽量避免 message 拷贝开销
		if (loop_->isInLocalThread())
		{
			sendInLocalLoop(message);
		}
		else
		{
			loop_->runInLocalThread(std::bind(&TcpConnection::sendInLocalLoop,
											  shared_from_this(), message));
		}
	}
}

void TcpConnection::sendInLocalLoop(const std::string& message)
{
	loop_->assertInLocalThread();
	if (state_ == Disconnected)
	{
		return;
	}

	size_t remaining = message.size();
	size_t n_wrote = 0;
	bool fault_error = false;

	// 先调用write尝试发送，将剩余的数据存放至output buffer
	if (!ch_->writing() && output_buffer_->readableBytes() == 0)
	{
		n_wrote = ::write(ch_->fd(), message.data(), message.size());
		if (n_wrote >= 0)
		{
			remaining -= n_wrote;
		}
		else
		{
			n_wrote = 0;
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				LOG_ERROR << "TcpConnection::send [" << errno
						  << "]: " << strerror(errno);
				handleError();
				fault_error = true;
			}
		}
	}

	if (!fault_error && remaining > 0)
	{
		output_buffer_->append(message.data() + n_wrote, remaining);
		if (!ch_->writing())
		{
			ch_->enableOUT();
		}

		if (output_buffer_->readableBytes() >= high_water_mark_ &&
			output_buffer_->readableBytes() - remaining < high_water_mark_ &&
			high_water_mark_callback_)
		{
			loop_->runInLocalThread(std::bind(high_water_mark_callback_,
											  shared_from_this(),
											  output_buffer_->readableBytes()));
		}
	}
	else if (!fault_error && remaining == 0)
	{
		if (write_complete_callback_)
		{
			loop_->runInLocalThread(
				std::bind(write_complete_callback_, shared_from_this()));
		}
	}
}

void TcpConnection::shutdown()
{
	if (state_ == Connected)
	{
		state_ = Disconnecting;
		loop_->runInLocalThread(
			std::bind(&TcpConnection::shutdownInLocalLoop, shared_from_this()));
	}
}

void TcpConnection::shutdownInLocalLoop()
{
	loop_->assertInLocalThread();
	assert(state_ == Disconnecting);

	if (!ch_->writing())
	{
		ch_->shutdownWR();
	}
}

void TcpConnection::establish()
{
	loop_->assertInLocalThread();
	assert(state_ == Connecting);
	state_ = Connected;

	ch_->tie(weak_from_this());
	ch_->enableIN();
	reading_ = true;
	ch_->useET();

	if (connect_callback_)
	{
		connect_callback_(shared_from_this());
	}
	LOG_DEBUG << "TcpConnection::establish - Connection " << name_
			  << " is fully establish.";
}

void TcpConnection::destroy()
{
	loop_->assertInLocalThread();
	if (state_ != Disconnected)
	{
		state_ = Disconnected;

		ch_->disableAll();

		if (connect_callback_)
		{
			connect_callback_(shared_from_this());
		}
	}

	ch_->remove();
	LOG_DEBUG << "TcpConnection::destroy - Connection " << name_
			  << " is fully destroyed.";
}

void TcpConnection::stopRead()
{
	if (state_ == Connected)
	{
		loop_->runInLocalThread(
			std::bind(&TcpConnection::stopReadInLocalLoop, shared_from_this()));
	}
}

void TcpConnection::stopReadInLocalLoop()
{
	loop_->assertInLocalThread();
	if (reading_)
	{
		ch_->disableIN();
		reading_ = false;
	}
}

void TcpConnection::startRead()
{
	if (state_ == Connected)
	{
		loop_->runInLocalThread(std::bind(&TcpConnection::startReadInLocalLoop,
										  shared_from_this()));
	}
}

void TcpConnection::startReadInLocalLoop()
{
	loop_->assertInLocalThread();
	if (!reading_)
	{
		ch_->enableIN();
		reading_ = true;
	}
}

void TcpConnection::forceClose()
{
	if (state_ == Connected || state_ == Disconnecting)
	{
		state_ = Disconnecting;
		loop_->runInLocalThread(std::bind(&TcpConnection::forceCloseInLocalLoop,
										  shared_from_this()));
	}
}

void TcpConnection::forceCloseInLocalLoop()
{
	loop_->assertInLocalThread();
	if (state_ == Connected || state_ == Disconnecting)
	{
		handleClose();
	}
}

} // namespace lynx