#ifndef LYNX_TCP_CLIENT_H
#define LYNX_TCP_CLIENT_H

#include "lynx/base/common.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <string>

namespace lynx
{
class EventLoop;
class TcpConnection;
class Buffer;
class Connector;
class TcpClient
{
  private:
	EventLoop* loop_;
	const std::string name_;
	std::shared_ptr<TcpConnection> conn_;
	std::shared_ptr<Connector> connector_;

	char serv_ip_[INET_ADDRSTRLEN];
	uint16_t serv_port_;

	int next_conn_id_;

	std::function<void(const std::shared_ptr<TcpConnection>&)>
		connection_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&,
					   std::shared_ptr<Buffer>)>
		message_callback_;
	std::function<void(const std::shared_ptr<TcpConnection>&)>
		write_complete_callback_;

  public:
	DISABLE_COPY(TcpClient)

	TcpClient(EventLoop* loop, const char* serv_ip, uint16_t serv_port,
			  const std::string name);
	~TcpClient();

	void connect();
	void disconnect();
	void stop();

	EventLoop* loop() const
	{
		return loop_;
	}

	const std::string& name() const
	{
		return name_;
	}

	void setConnectionCallback(
		const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb)
	{
		connection_callback_ = cb;
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
	void handleNewConnection(int fd);
	void handleClose(const std::shared_ptr<TcpConnection>& conn);
};

} // namespace lynx

#endif