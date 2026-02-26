#ifndef LYNX_TCP_CONNECTION_HPP
#define LYNX_TCP_CONNECTION_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
namespace lynx
{
namespace tcp
{
class Channel;
class EventLoop;
class Buffer;
class Connection : public base::noncopyable,
				   public std::enable_shared_from_this<Connection>
{
  private:
	enum class State
	{
		kDisconnected,
		kConnecting,
		kConnected,
		kDisconnecting
	} state_;
	// establish: Connecting -> Connected
	// shutdown: Connected -> Disconnecting
	// handleClose: Connected/Disconnecting -> Disconnected
	// destroy: [Connected/Disconnecting -> Disconnected]

	EventLoop* loop_;
	std::unique_ptr<Channel> ch_;
	InetAddr addr_;
	uint64_t seq_;

	size_t high_water_mark_;
	std::unique_ptr<Buffer> inbuf_;
	std::unique_ptr<Buffer> outbuf_;

	std::any ctx_;

	int file_fd_{-1};
	size_t file_bytes_to_send_{0};
	off_t file_offset_{0};

	std::function<void(const std::shared_ptr<Connection>&,
					   Buffer*)>
		message_callback_; // defined by user
	std::function<void(const std::shared_ptr<Connection>&)>
		write_complete_callback_; // defined by user, executed in
								  // pendingfunctors
	std::function<void(const std::shared_ptr<Connection>&)>
		close_callback_; // provided by "Server"
	std::function<void(const std::shared_ptr<Connection>&)>
		connect_callback_; // for read and close, defined by user. Only called
						   // when established or destroyed

	std::function<void(const std::shared_ptr<Connection>&, size_t)>
		high_water_mark_callback_;

  public:
	Connection(int fd, EventLoop* loop, const InetAddr& addr, uint64_t seq);
	~Connection();

	int fd() const;

	uint64_t seq() const
	{
		return seq_;
	}

	bool connected() const
	{
		return state_ == State::kConnected;
	}

	bool connecting() const
	{
		return state_ == State::kConnecting;
	}

	bool disconnected() const
	{
		return state_ == State::kDisconnected;
	}

	bool disconnecting() const
	{
		return state_ == State::kDisconnecting;
	}

	EventLoop* loop() const
	{
		return loop_;
	}

	const InetAddr& addr() const
	{
		return addr_;
	}

	InetAddr& addr()
	{
		return addr_;
	}

	void setMessageCallback(
		std::function<void(const std::shared_ptr<Connection>&, Buffer*)> cb)
	{
		message_callback_ = std::move(cb);
	}

	void setWriteCompleteCallback(
		std::function<void(const std::shared_ptr<Connection>&)> cb)
	{
		write_complete_callback_ = std::move(cb);
	}

	void setConnectCallback(
		std::function<void(const std::shared_ptr<Connection>&)> cb)
	{
		connect_callback_ = std::move(cb);
	}

	void setCloseCallback(
		std::function<void(const std::shared_ptr<Connection>&)> cb)
	{
		close_callback_ = std::move(cb);
	}

	void setHighWaterMarkCallback(
		std::function<void(const std::shared_ptr<Connection>&, size_t)> cb,
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

	void setContext(const std::any& ctx)
	{
		ctx_ = ctx;
	}

	std::any context() const
	{
		return ctx_;
	}

	std::any* contextPtr()
	{
		return &ctx_;
	}

	void connEstablish();
	void connDestroy();

	void sendFile(const std::string& file_path);

  private:
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void sendInLoop(const std::string& message);
	void shutdownInLoop();

	void sendFileInLoop(const std::string& file_path);
	void trySendFile();
};
} // namespace tcp
} // namespace lynx

#endif