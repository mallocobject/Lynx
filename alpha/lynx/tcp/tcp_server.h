#ifndef LYNX_TCP_SERVER_H
#define LYNX_TCP_SERVER_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
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

  public:
	TcpServer(EventLoop* loop, const InetAddr& addr, const std::string& name,
			  size_t sub_reactor_num);
	~TcpServer();

	void run();
	void setConnectionCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb)
	{
		connect_callback_ = cb;
	}

	void setMessageCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&,
								 Buffer*)>& cb)
	{
		message_callback_ = cb;
	}

	void setWriteCompleteCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb)
	{
		write_complete_callback_ = cb;
	}

  private:
	void handleNewConnection(int conn_fd, const InetAddr& addr);
	void handleClose(const std::shared_ptr<TcpConnection>& conn);
	void handleCloseInLoop(const std::shared_ptr<TcpConnection>& conn);
};
} // namespace lynx

#endif