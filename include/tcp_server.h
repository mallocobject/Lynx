#ifndef LYNX_TCP_SERVER_H
#define LYNX_TCP_SERVER_H

#include "lynx/include/common.h"
#include "lynx/include/time_stamp.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <unordered_map>
namespace lynx
{
class EventLoop;
class Acceptor;
class TcpConnection;
class EventLoopThreadPool;
class Buffer;
class TcpServer
{
  private:
	EventLoop* main_reactor_;
	const std::string name_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> sub_reactors;
	std::unordered_map<std::string, std::shared_ptr<TcpConnection>> conn_map_;
	char ip_[INET_ADDRSTRLEN];
	uint16_t port_;
	bool running;
	int next_conn_id_;

	std::function<void(const std::shared_ptr<TcpConnection>&)>
		connect_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&,
					   std::shared_ptr<Buffer>)>
		message_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		write_complete_callback_;

  public:
	DISABLE_COPY(TcpServer)

	TcpServer(EventLoop* loop, const char* ip, uint16_t port,
			  const std::string& name,
			  size_t sub_reactor_num = std::thread::hardware_concurrency());
	~TcpServer();

	void startup();
	void setConnectionCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb)
	{
		connect_callback_ = cb;
	}

	void setMessageCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&,
								 std::shared_ptr<Buffer>)>& cb)
	{
		message_callback_ = cb;
	}

	void setWriteCompleteCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb)
	{
		write_complete_callback_ = cb;
	}

  private:
	void handleNewConnection(int conn_fd, const char* peer_ip,
							 uint16_t peer_port);

	void handleClose(const std::shared_ptr<TcpConnection>& conn);
	void handleCloseInLocalLoop(const std::shared_ptr<TcpConnection>& conn);
};
} // namespace lynx

#endif