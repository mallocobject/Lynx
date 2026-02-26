#ifndef LYNX_TCP_SERVER_HPP
#define LYNX_TCP_SERVER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
namespace lynx
{
namespace tcp
{
class EventLoop;
class Acceptor;
class Connection;
class EventLoopThreadPool;
class Buffer;
class Server : public base::noncopyable
{
  private:
	EventLoop* main_reactor_;
	const std::string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> sub_reactor_pool_;
	std::map<uint64_t, std::shared_ptr<Connection>> conn_map_;

	std::atomic<uint64_t> seq_;

	std::function<void(const std::shared_ptr<Connection>&)>
		connect_callback_; // call it when the state is changed
	std::function<void(const std::shared_ptr<Connection>&, Buffer*)>
		message_callback_;
	std::function<void(const std::shared_ptr<Connection>&)>
		write_complete_callback_;
	std::function<void(const std::shared_ptr<Connection>&, size_t)>
		high_water_mark_callback_;
	size_t high_water_mark_;

  public:
	Server(EventLoop* loop, const InetAddr& addr, const std::string& name,
		   size_t sub_reactor_num);
	Server(EventLoop* loop, const std::string& ip, uint16_t port,
		   const std::string& name, size_t sub_reactor_num);
	~Server();

	size_t connectionNum() const
	{
		return conn_map_.size();
	}

	void run();
	void setConnectionCallback(
		std::function<void(const std::shared_ptr<Connection>&)> cb)
	{
		connect_callback_ = std::move(cb);
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

	void setHighWaterMarkCallback(
		std::function<void(const std::shared_ptr<Connection>&, size_t)> cb,
		size_t high_water_mark)
	{
		high_water_mark_callback_ = std::move(cb);
		high_water_mark_ = high_water_mark;
	}

  private:
	void handleNewConnection(int conn_fd, const InetAddr& addr);
	void handleClose(const std::shared_ptr<Connection>& conn);
	void handleCloseInLoop(const std::shared_ptr<Connection>& conn);
};
} // namespace tcp
} // namespace lynx

#endif