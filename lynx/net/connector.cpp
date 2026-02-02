#include "lynx/net/connector.h"
#include "lynx/logger/logger.h"
#include "lynx/net/channel.h"
#include "lynx/net/event_loop.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace lynx
{

static const int MaxRetryDelayMs = 30 * 1e3;
static const int InitRetryDelayMs = 500;

Connector::Connector(EventLoop* loop, const char* serv_ip, uint16_t serv_port)
	: loop_(loop), serv_port_(serv_port), connect_(false), state_(Disconnected),
	  retry_delay_ms_(InitRetryDelayMs)
{
	strcpy(serv_ip_, serv_ip);

	LOG_DEBUG << "Connector created [" << this << ']';
}

Connector::~Connector()
{
	LOG_DEBUG << "Connector destroyed [" << this << ']';
}

void Connector::startup()
{
	connect_ = true;
	loop_->runInLocalThread(
		std::bind(&Connector::startInLocalLoop, shared_from_this()));
}

void Connector::restart()
{
	loop_->assertInLocalThread();
	state_ = Disconnected;
	retry_delay_ms_ = InitRetryDelayMs;
	connect_ = true;
	startInLocalLoop();
}

void Connector::startInLocalLoop()
{
	loop_->assertInLocalThread();
	assert(!ch_);
	assert(state_ == Disconnected);
	if (connect_)
	{
		connect();
	}
	else
	{
		LOG_DEBUG << "don't connect";
	}
}

void Connector::stop()
{
	connect_ = false;
	loop_->queueInLocalThread(
		std::bind(&Connector::stopInLocalLoop, shared_from_this()));
}

void Connector::stopInLocalLoop()
{
	loop_->assertInLocalThread();
	if (state_ == Connecting)
	{
		state_ = Disconnected;
		removeAndResetChannel();
	}
}

void Connector::removeAndResetChannel()
{
	assert(ch_);
	ch_->disableAll();
	ch_->remove();
	ch_.reset();
}

void Connector::connect()
{
	int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

	assert(!ch_);
	ch_ = std::make_unique<Channel>(fd, loop_);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serv_port_);
	::inet_pton(AF_INET, serv_ip_, &addr.sin_addr);

	// int ret = ::connect(fd, (sockaddr*)&addr,
	// 					sizeof(sockaddr)); // has not create channel

	int saved_errno = 0;
	ch_->connect((sockaddr*)&addr, &saved_errno);

	switch (saved_errno)
	{
	case 0:			  // 立即成功
	case EINPROGRESS: // 正在连接中
	case EINTR:		  // 被信号中断
	case EISCONN:	  // 套接字已连接
		connecting();
		break;

	case EAGAIN:		// 临时资源不足
	case EADDRINUSE:	// 本地地址/端口已被占用
	case EADDRNOTAVAIL: // 无可用临时端口
	case ECONNREFUSED:	// 服务器拒绝连接
	case ENETUNREACH:	// 网络不可达
		retry();
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_ERROR << "connect error in Connector::startInLoop "
				  << strerror(saved_errno);
		ch_.reset();
		break;

	default:
		LOG_ERROR << "Unexpected error in Connector::startInLoop "
				  << strerror(saved_errno);
		ch_.reset();
		// connectErrorCallback_();
		break;
	}
}

void Connector::connecting()
{
	state_ = Connecting;
	ch_->setWriteCallback(
		std::bind(&Connector::handleWrite, shared_from_this()));
	ch_->setErrorCallback(
		std::bind(&Connector::handleError, shared_from_this()));

	ch_->enableOUT(); // 监听 TCP 连接可写事件
}

void Connector::handleWrite()
{
	if (state_ == Connecting)
	{
		int error = ch_->getSocketError();

		if (error)
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = " << error << " "
					 << strerror(error);
			retry();
		}
		else if (isSelfConnect())
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry();
		}
		else
		{
			state_ = Connected;
			if (connect_)
			{
				int fd = ch_->releaseFd();
				removeAndResetChannel();
				if (new_connection_callback)
				{
					new_connection_callback(fd);
				}
			}
			else
			{
				removeAndResetChannel();
			}
		}
	}
}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError state=" << state_;
	if (state_ == Connecting)
	{
		int error = ch_->getSocketError();
		LOG_ERROR << "TcpConnection::handleError - SO_ERROR = " << error << " "
				  << strerror(error);
	}
}

void Connector::retry()
{
	state_ = Disconnected;
	if (connect_)
	{
		if (ch_)
		{
			removeAndResetChannel();
		}
		LOG_INFO << "Connector::retry - Retry connecting to " << serv_ip_ << ':'
				 << serv_port_;
		loop_->runAfter(
			retry_delay_ms_ / 1000.0,
			std::bind(&Connector::startInLocalLoop, shared_from_this()));
		retry_delay_ms_ = std::min(retry_delay_ms_ * 2, MaxRetryDelayMs);
	}
	else
	{
		LOG_DEBUG << "don't connect";
	}
}

bool Connector::isSelfConnect() const
{
	assert(ch_);

	sockaddr_in local_addr;
	sockaddr_in peer_addr;
	::bzero(&local_addr, sizeof(sockaddr_in));
	::bzero(&peer_addr, sizeof(sockaddr_in));

	socklen_t addr_len = sizeof(sockaddr_in);

	if (::getsockname(ch_->fd(), (sockaddr*)&local_addr, &addr_len) < 0)
	{
		return false;
	}

	if (::getpeername(ch_->fd(), (sockaddr*)&peer_addr, &addr_len) < 0)
	{
		return false;
	}

	return local_addr.sin_family == AF_INET &&
		   local_addr.sin_addr.s_addr == peer_addr.sin_addr.s_addr &&
		   local_addr.sin_port == peer_addr.sin_port;
}

} // namespace lynx