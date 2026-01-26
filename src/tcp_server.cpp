#include "lynx/include/tcp_server.h"
#include "lynx/include/acceptor.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/event_loop_thread_pool.h"
#include "lynx/include/logger.hpp"
#include "lynx/include/tcp_connection.h"
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>

namespace lynx
{
TcpServer::TcpServer(EventLoop* loop, const char* ip, uint16_t port,
					 const std::string& name, size_t sub_reactor_num)
	: main_reactor_(loop), name_(name),
	  acceptor_(std::make_unique<Acceptor>(loop, ip, port)),
	  sub_reactors(
		  std::make_unique<EventLoopThreadPool>(loop, sub_reactor_num)),
	  port_(port), running(false), next_conn_id_(1)

{
	strcpy(ip_, ip);
	acceptor_->setNewConnectionCallback(
		std::bind(&TcpServer::handleNewConnection, this, std::placeholders::_1,
				  std::placeholders::_2, std::placeholders::_3));
}

TcpServer::~TcpServer()
{
	main_reactor_->assertInLocalThread();
	LOG_INFO() << "TcpServer::~TcpServer [" << name_ << "] is a shutting down";

	for (auto& item : conn_map_)
	{
		std::shared_ptr<TcpConnection> conn(item.second);

		item.second.reset();

		// 注销回调函数，因为服务器要关闭了
		conn->setConnectCallback(nullptr);
		conn->setMessageCallback(nullptr);
		conn->setCloseCallback(nullptr);
		conn->setWriteCompleteCallback(nullptr);

		conn->loop()->runInLocalThread(
			std::bind(&TcpConnection::destroy, conn));
	}

	LOG_INFO() << "TcpServer::~TcpServer [" << name_
			   << "] has dispatched all cleanup tasks";
}

void TcpServer::handleNewConnection(int conn_fd, const char* peer_ip,
									uint16_t peer_port)
{
	main_reactor_->assertInLocalThread();
	char buf[64];
	snprintf(buf, sizeof(buf), "-%s:%d#%d", peer_ip, peer_port,
			 next_conn_id_++);
	std::string conn_name = name_ + buf;

	LOG_INFO() << "TcpServer::handleNewConnection [" << name_
			   << "] - new connection [" << conn_name << "] from " << peer_ip
			   << ':' << peer_port;

	EventLoop* io_loop = sub_reactors->getNextLoop();
	std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(
		conn_fd, io_loop, conn_name, ip_, port_, peer_ip, peer_port);
	conn_map_[conn_name] = conn;
	conn->setConnectCallback(connection_callback_);
	conn->setMessageCallback(message_callback_);
	conn->setWriteCompleteCallback(write_complete_callback_);
	conn->setCloseCallback(
		std::bind(&TcpServer::handleClose, this, std::placeholders::_1));

	io_loop->runInLocalThread(std::bind(&TcpConnection::establish, conn));
}

void TcpServer::handleClose(const std::shared_ptr<TcpConnection>& conn)
{
	main_reactor_->runInLocalThread(
		std::bind(&TcpServer::handleCloseInLocalLoop, this, conn));
}

void TcpServer::handleCloseInLocalLoop(
	const std::shared_ptr<TcpConnection>& conn)
{
	main_reactor_->assertInLocalThread();
	// LOG_DEBUG() << "TcpServer::handleCloseInLocalLoop [" << name_
	// 			<< "] - connection " << conn->name();

	decltype(conn_map_)::iterator iter = conn_map_.find(conn->name());
	assert(iter != conn_map_.end());
	conn_map_.erase(iter);

	// 销毁动作一定发生在子线程处理完当前连接的所有存量事件之后
	conn->loop()->runInLocalThread(std::bind(&TcpConnection::destroy, conn));
}

void TcpServer::startup()
{
	sub_reactors->startup();
	acceptor_->listen();
	main_reactor_->run();
}
} // namespace lynx