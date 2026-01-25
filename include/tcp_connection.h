#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "lynx/include/common.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <utility>
namespace lynx
{
class Channel;
class EventLoop;
class Buffer;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
  private:
	enum State
	{
		Disconnected,
		Connecting,
		Connected,
		Disconnecting
	} state_;
	// create: Disconnected -> Connecting
	// ready: Connecting -> Connected
	// other: Connected -> Disconnected

	int fd_;
	std::unique_ptr<Channel> ch_;
	EventLoop* loop_;
	std::string name_;
	char peer_ip_[INET_ADDRSTRLEN]; // include '\0'
	char local_ip_[INET_ADDRSTRLEN];
	uint16_t peer_port_;
	uint16_t local_port_;
	std::shared_ptr<Buffer> input_buffer_;
	std::shared_ptr<Buffer> output_buffer_;

	std::function<void(const std::shared_ptr<TcpConnection>&)>
		message_callback_; // defined by user
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		write_complete_callback_; // defined by user
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		close_callback_; // provided by "TcpServer"
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		connect_callback_; // for read and close, defined by user

	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

  public:
	DISABLE_COPY(TcpConnection)

	TcpConnection(int fd, EventLoop* loop, const std::string& name,
				  const char* local_ip, uint16_t local_port,
				  const char* peer_ip, uint16_t peer_port);
	~TcpConnection();

	int fd() const
	{
		return fd_;
	}

	const std::string name() const
	{
		return name_;
	}

	EventLoop* loop() const
	{
		return loop_;
	}

	const char* peerIp() const
	{
		return peer_ip_;
	}

	uint16_t peerPort() const
	{
		return peer_port_;
	}

	void setMessageCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		message_callback_ = std::move(cb);
	}

	void setWriteCompleteCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		write_complete_callback_ = std::move(cb);
	}

	void setConnectCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		connect_callback_ = std::move(cb);
	}

	void setCloseCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		close_callback_ = std::move(cb);
	}

	void setTcpReuseAddr(bool on);
	void setTcpNoDelay(bool on);

	void send(const std::string& message);

	std::shared_ptr<Buffer> inputBuffer() const
	{
		return input_buffer_;
	}

	std::shared_ptr<Buffer> outputBuffer() const
	{
		return output_buffer_;
	}

	void establish();
	void destroy();

  private:
	void sendInLocalLoop(const std::string& message);
};
} // namespace lynx

#endif