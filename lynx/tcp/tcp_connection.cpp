#include "lynx/tcp/tcp_connection.h"
#include "lynx/logger/logger.h"
#include "lynx/tcp/buffer.h"
#include "lynx/tcp/channel.h"
#include "lynx/tcp/event_loop.h"
#include "lynx/tcp/inet_addr.hpp"
#include "lynx/tcp/socket.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace lynx;

TcpConnection::TcpConnection(int fd, EventLoop* loop, const InetAddr& addr,
							 uint64_t seq)
	: loop_(loop), addr_(addr), seq_(seq), state_(State::kConnecting),
	  high_water_mark_(64 * 1024 * 1024)
{
	Socket::setKeepAlive(fd);
	Socket::setNoDelay(fd);
	ch_ = std::make_unique<Channel>(fd, loop);

	ch_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	ch_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	ch_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	ch_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	inbuf_ = std::make_unique<Buffer>();
	outbuf_ = std::make_unique<Buffer>();
}

TcpConnection::~TcpConnection()
{
	if (file_fd_ != -1)
	{
		::close(file_fd_);
	}
}

void TcpConnection::send(const std::string& message)
{
	if (state_ == State::kConnected)
	{
		// 手动判断，尽量避免 message 拷贝开销
		if (loop_->InLoopThread())
		{
			sendInLoop(message);
		}
		else
		{
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
									   shared_from_this(), message));
		}
	}
}

void TcpConnection::sendInLoop(const std::string& message)
{
	loop_->assertInLoopThread();
	if (state_ == State::kDisconnected)
	{
		return;
	}

	size_t remaining = message.size();
	size_t n_wrote = 0;
	bool fault_error = false;

	// 先调用write尝试发送，将剩余的数据存放至output buffer
	if (!ch_->writing() && outbuf_->readableBytes() == 0)
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
				LOG_ERROR << "write failed: " << strerror(errno);
				handleError();
				fault_error = true;
			}
		}
	}

	if (!fault_error && remaining > 0)
	{
		outbuf_->append(message.data() + n_wrote, remaining);
		if (!ch_->writing())
		{
			ch_->enableOUT();
		}

		if (outbuf_->readableBytes() >= high_water_mark_ &&
			outbuf_->readableBytes() - remaining < high_water_mark_ &&
			high_water_mark_callback_)
		{
			loop_->queueInLoop(std::bind(high_water_mark_callback_,
										 shared_from_this(),
										 outbuf_->readableBytes()));
		}
	}
	else if (!fault_error && remaining == 0)
	{
		if (write_complete_callback_)
		{
			loop_->queueInLoop(
				std::bind(write_complete_callback_, shared_from_this()));
		}
	}
}

void TcpConnection::shutdown()
{
	if (state_ == State::kConnected)
	{
		state_ = State::kDisconnecting;
		loop_->runInLoop(
			std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
	}
}

void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	assert(state_ == State::kDisconnecting);

	if (!ch_->writing())
	{
		Socket::shutdown(ch_->fd());
	}
}

void TcpConnection::connEstablish()
{
	loop_->assertInLoopThread();
	assert(state_ == State::kConnecting);
	state_ = State::kConnected;

	ch_->tie(weak_from_this());
	ch_->enableIN();

	if (connect_callback_)
	{
		connect_callback_(shared_from_this());
	}
	LOG_TRACE << "Connection form " << addr_.toFormattedString()
			  << " is fully establish.";
}

void TcpConnection::connDestroy()
{
	loop_->assertInLoopThread();
	if (state_ != State::kDisconnected)
	{
		state_ = State::kDisconnected;

		ch_->disableAll();

		if (connect_callback_)
		{
			connect_callback_(shared_from_this());
		}
	}

	ch_->remove();
	LOG_DEBUG << "Connection form " << addr_.toFormattedString()
			  << " is fully destroyed.";
}

void TcpConnection::sendFile(const std::string& file_path)
{
	if (state_ == State::kConnected)
	{
		if (loop_->InLoopThread())
		{
			sendFileInLoop(file_path);
		}
		else
		{
			loop_->runInLoop(std::bind(&TcpConnection::sendFileInLoop,
									   shared_from_this(), file_path));
		}
	}
}

void TcpConnection::sendFileInLoop(const std::string& file_path)
{
	loop_->assertInLoopThread();
	LOG_TRACE << "start send file";
	if (state_ == State::kDisconnected)
	{
		return;
	}

	if (file_fd_ != -1)
	{
		LOG_WARN << "TcpConnection::sendFile: a file has been send.";
		return;
	}
	int fd = ::open(file_path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		LOG_WARN << "cannot open file: " << file_path;
		return;
	}
	struct stat st;
	if (::fstat(fd, &st) < 0)
	{
		LOG_ERROR << "cannot get the file's state.";
		::close(fd);
		return;
	}

	file_fd_ = fd;
	file_bytes_to_send_ = st.st_size;
	file_offset_ = 0;

	if (!ch_->writing())
	{
		ch_->enableOUT();
	}

	if (outbuf_->readableBytes() == 0)
	{
		trySendFile();
	}
}

void TcpConnection::trySendFile()
{
	loop_->assertInLoopThread();
	bool fault_error = false;

	while (file_bytes_to_send_ > 0)
	{
		ssize_t n =
			::sendfile(ch_->fd(), file_fd_, &file_offset_, file_bytes_to_send_);
		if (n >= 0)
		{
			file_bytes_to_send_ -= n;
		}
		else
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				if (!ch_->writing())
				{
					ch_->enableOUT();
				}
				break;
			}
			else
			{
				LOG_ERROR << "try send file failed - fd " << ch_->fd() << ": "
						  << strerror(errno);

				::close(file_fd_);
				file_fd_ = -1;
				handleError();
				return;
			}
		}
	}

	if (file_bytes_to_send_ == 0)
	{
		::close(file_fd_);
		file_fd_ = -1;
		LOG_DEBUG << "TcpConnection::trySendFile : file send successfully.";
	}
}

void TcpConnection::handleError()
{
	int error = Socket::socketErrno(ch_->fd());

	if (error == 0 || error == -1)
	{
		return;
	}

	else if (error == ECONNRESET)
	{
		LOG_WARN << "Client disconnected unexpectedly RESET on "
				 << addr_.toFormattedString();
	}
	else
	{
		LOG_ERROR << "error occupied on " << addr_.toFormattedString() << ' '
				  << strerror(error);
	}
}

void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();
	if (state_ == State::kDisconnected)
	{
		return;
	}
	LOG_TRACE << "Connection closed - FD: " << ch_->fd()
			  << ", Peer: " << addr_.toFormattedString();
	assert(state_ == State::kConnected || state_ == State::kDisconnecting);
	state_ = State::kDisconnected;
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
	loop_->assertInLoopThread();
	int saved_errno = 0;
	ssize_t n = inbuf_->readFd(ch_->fd(), &saved_errno);
	if (n > 0)
	{
		message_callback_(shared_from_this(), inbuf_.get());
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		handleError();
	}
}

void TcpConnection::handleWrite()
{
	loop_->assertInLoopThread();

	if (ch_->writing())
	{
		if (outbuf_->readableBytes() > 0)
		{
			ssize_t n =
				::write(ch_->fd(), outbuf_->peek(), outbuf_->readableBytes());
			if (n > 0)
			{
				outbuf_->retrieve(n);
			}
			else
			{
				if (errno != EWOULDBLOCK && errno != EAGAIN)
				{
					handleError();
				}
			}
		}

		if (outbuf_->readableBytes() == 0 && file_fd_ != -1)
		{
			trySendFile();
		}

		if (outbuf_->readableBytes() == 0 && file_fd_ == -1)
		{

			ch_->disableOUT();
			if (write_complete_callback_)
			{
				loop_->queueInLoop(
					std::bind(write_complete_callback_, shared_from_this()));
			}

			if (state_ == State::kDisconnecting)
			{
				shutdownInLoop();
			}
		}
	}
}