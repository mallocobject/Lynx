#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "lynx/include/common.h"
#include <any>
#include <cstddef>
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
	// establish: Connecting -> Connected
	// shutdown: Connected -> Disconnecting
	// handleColse: Connected/Disconnecting -> Disconnected
	// destroy: [Connected/Disconnecting -> Disconnected]

	int fd_;
	std::unique_ptr<Channel> ch_;
	EventLoop* loop_;
	std::string name_;
	char peer_ip_[INET_ADDRSTRLEN]; // include '\0'
	char local_ip_[INET_ADDRSTRLEN];
	uint16_t peer_port_;
	uint16_t local_port_;
	size_t high_water_mark_;
	bool reading_;
	std::shared_ptr<Buffer> input_buffer_;
	std::shared_ptr<Buffer> output_buffer_;

	std::any context_;

	int file_fd_ = -1;
	size_t file_bytes_to_send_ = 0;
	off_t file_offset_ = 0;

	std::function<void(const std::shared_ptr<TcpConnection>&,
					   std::shared_ptr<Buffer>)>
		message_callback_; // defined by user
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		write_complete_callback_; // defined by user, executed in
								  // pendingfunctors
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		close_callback_; // provided by "TcpServer"
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		connect_callback_; // for read and close, defined by user. Only called
						   // when established or destroyed

	std::function<void(const std::shared_ptr<TcpConnection>&, size_t)>
		high_water_mark_callback_;

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

	std::string name() const
	{
		return name_;
	}

	bool connected() const
	{
		return state_ == Connected;
	}

	bool connecting() const
	{
		return state_ == Connecting;
	}

	bool disconnecting() const
	{
		return state_ == Disconnecting;
	}

	bool disconnnected() const
	{
		return state_ == Disconnected;
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
		std::function<void(const std::shared_ptr<TcpConnection>&,
						   std::shared_ptr<Buffer>)>
			cb)
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

	void setHighWaterMarkCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&, size_t)> cb,
		size_t high_water_mark)
	{
		high_water_mark_callback_ = std::move(cb);
		high_water_mark_ = high_water_mark;
	}

	void setTcpReuseAddr(bool on);
	void setTcpKeepAlive(bool on);
	void setTcpNoDelay(bool on);

	void send(const std::string& message);
	void shutdown();

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

	bool reading() const
	{
		return reading_;
	}

	void stopRead();
	void startRead();

	void forceClose();

	void setContext(const std::any& context)
	{
		context_ = context;
	}

	std::any context() const
	{
		return context_;
	}

	std::any* contextPtr()
	{
		return &context_;
	}

	void sendFile(const std::string& file_path);

  private:
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void sendInLocalLoop(const std::string& message);
	void shutdownInLocalLoop();

	void stopReadInLocalLoop();
	void startReadInLocalLoop();

	void forceCloseInLocalLoop();

	void sendFileInLocalLoop(const std::string& file_path);

	void trySendFile();
};
} // namespace lynx

#endif