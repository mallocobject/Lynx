#ifndef LYNX_PUBLIC_TCP_SERVER_HPP
#define LYNX_PUBLIC_TCP_SERVER_HPP

#include "inet_addr.hpp"
#include "noncopyable.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
namespace lynx
{
class EventLoop;
class Acceptor;
class TcpConnection;
class EventLoopThreadPool;
class Buffer;
class TcpServer : public noncopyable
{
  private:
	EventLoop* main_reactor_;
	const std::string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> sub_reactor_pool_;
	std::unordered_map<uint64_t, std::shared_ptr<TcpConnection>> conn_map_;

	std::atomic<uint64_t> seq_;

	std::function<void(const std::shared_ptr<TcpConnection>&)>
		connect_callback_; // call it when the state is changed
	std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>
		message_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		write_complete_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&, size_t)>
		high_water_mark_callback_;
	size_t high_water_mark_;

  public:
	TcpServer(EventLoop* loop, const InetAddr& addr, const std::string& name,
			  size_t sub_reactor_num);
	TcpServer(EventLoop* loop, const std::string& ip, uint16_t port,
			  const std::string& name, size_t sub_reactor_num);
	~TcpServer();

	size_t connectionNum() const
	{
		return conn_map_.size();
	}

	void run();
	void setConnectionCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		connect_callback_ = std::move(cb);
	}

	void setMessageCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)> cb)
	{
		message_callback_ = std::move(cb);
	}

	void setWriteCompleteCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&)> cb)
	{
		write_complete_callback_ = std::move(cb);
	}

	void setHighWaterMarkCallback(
		std::function<void(const std::shared_ptr<TcpConnection>&, size_t)> cb,
		size_t high_water_mark)
	{
		high_water_mark_callback_ = std::move(cb);
		high_water_mark_ = high_water_mark;
	}

  private:
	void handleNewConnection(int conn_fd, const InetAddr& addr);
	void handleClose(const std::shared_ptr<TcpConnection>& conn);
	void handleCloseInLoop(const std::shared_ptr<TcpConnection>& conn);
};
} // namespace lynx

#endif