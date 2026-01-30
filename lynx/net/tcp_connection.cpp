#include "lynx/net/tcp_connection.h"
#include "lynx/base/logger.hpp"
#include "lynx/net/buffer.h"
#include "lynx/net/channel.h"
#include "lynx/net/event_loop.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <strings.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
	ch_->setNoDelay(true);

	ch_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	ch_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	ch_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	ch_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
	if (file_fd_ != -1)
	{
		::close(file_fd_);
	}
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
	if (error == ECONNRESET)
	{
		LOG_INFO << "TcpConnection::handleError [" << name_
				 << "] = Connection reset by peer (Normal under high load)";
	}
	else
	{
		LOG_ERROR << "TcpConnection::handleError [" << name_
				  << "] = SO_ERROR = " << error << ": " << strerror(error);
	}
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
		if (output_buffer_->readableBytes() > 0)
		{
			ssize_t n = ::write(fd_, output_buffer_->peek(),
								output_buffer_->readableBytes());
			if (n > 0)
			{
				output_buffer_->retrieve(n);
			}
			else
			{
				if (errno != EWOULDBLOCK && errno != EAGAIN)
				{
					LOG_ERROR << "TcpConnection::handleWrite: error";
					handleError();
				}
			}
		}

		if (output_buffer_->readableBytes() == 0 && file_fd_ != -1)
		{
			trySendFile();
		}

		if (output_buffer_->readableBytes() == 0 && file_fd_ == -1)
		{

			ch_->disableOUT();
			if (write_complete_callback_)
			{
				loop_->queueInLocalThread(
					std::bind(write_complete_callback_, shared_from_this()));
			}

			if (state_ == Disconnecting)
			{
				shutdownInLocalLoop();
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
			loop_->queueInLocalThread(
				std::bind(high_water_mark_callback_, shared_from_this(),
						  output_buffer_->readableBytes()));
		}
	}
	else if (!fault_error && remaining == 0)
	{
		if (write_complete_callback_)
		{
			loop_->queueInLocalThread(
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
	LOG_INFO << "TcpConnection::establish - Connection " << name_
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
		loop_->queueInLocalThread(std::bind(
			&TcpConnection::forceCloseInLocalLoop, shared_from_this()));
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

void TcpConnection::sendFile(const std::string& file_path)
{
	if (state_ == Connected)
	{

		if (loop_->isInLocalThread())
		{
			sendFileInLocalLoop(file_path);
		}
		else
		{
			loop_->runInLocalThread(
				std::bind(&TcpConnection::sendFileInLocalLoop,
						  shared_from_this(), file_path));
		}
	}
}

void TcpConnection::sendFileInLocalLoop(const std::string& file_path)
{
	loop_->assertInLocalThread();
	if (state_ == Disconnected)
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
		LOG_WARN << "TcpConnection::sendFile: cannot open file: " << file_path;
		return;
	}
	struct stat st;
	if (::fstat(fd, &st) < 0)
	{
		LOG_ERROR << "TcpConnection::sendFile: cannot get the file's state.";
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

	if (output_buffer_->readableBytes() == 0)
	{
		trySendFile();
	}
}

void TcpConnection::trySendFile()
{
	loop_->assertInLocalThread();
	bool fault_error = false;

	while (file_bytes_to_send_ > 0)
	{
		ssize_t n =
			::sendfile(fd_, file_fd_, &file_offset_, file_bytes_to_send_);
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
				LOG_ERROR << "TcpConnection::trySendFile [" << errno
						  << "]: " << strerror(errno);

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

} // namespace lynx