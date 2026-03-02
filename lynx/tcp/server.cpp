#include "lynx/tcp/server.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/acceptor.hpp"
#include "lynx/tcp/connection.hpp"
#include "lynx/tcp/event_loop.hpp"
#include "lynx/tcp/event_loop_thread_pool.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <atomic>
#include <csignal>
#include <functional>
#include <memory>
#include <strings.h>

using namespace lynx;
using namespace lynx::tcp;

Server::Server(EventLoop* loop, const InetAddr& addr, const std::string& name,
			   size_t sub_reactor_num)
	: main_reactor_(loop), name_(name), seq_(0)
{
	static bool ignored = []()
	{
		struct sigaction sa;
		::bzero(&sa, sizeof(sa));
		sa.sa_handler = SIG_IGN;
		sa.sa_flags = 0;
		::sigemptyset(&sa.sa_mask);
		::sigaction(SIGPIPE, &sa, nullptr);
		return true;
	}();

	sub_reactor_pool_ =
		std::make_unique<EventLoopThreadPool>(loop, sub_reactor_num);

	acceptor_ = std::make_unique<Acceptor>(loop, addr);
	acceptor_->setNewConnectionCallback(std::bind(&Server::handleNewConnection,
												  this, std::placeholders::_1,
												  std::placeholders::_2));
}

Server::Server(EventLoop* loop, const std::string& ip, uint16_t port,
			   const std::string& name, size_t sub_reactor_num)
	: Server(loop, InetAddr(ip, port), name, sub_reactor_num)
{
}

Server::~Server()
{
	main_reactor_->assertInLoopThread();
	LOG_TRACE << "Server::~Server [" << name_ << "] is a shutting down";

	for (auto& item : conn_map_)
	{
		std::shared_ptr<Connection> conn(item.second);

		item.second.reset();

		// 注销回调函数，因为服务器要关闭了
		conn->setConnectCallback(nullptr);
		conn->setMessageCallback(nullptr);
		conn->setCloseCallback(nullptr);
		conn->setWriteCompleteCallback(nullptr);
		conn->setHighWaterMarkCallback(nullptr, 0);

		conn->loop()->runInLoop(std::bind(&Connection::connDestroy, conn));
	}

	LOG_TRACE << "Server: " << name_ << " has dispatched all cleanup tasks";
}

void Server::run()
{
	sub_reactor_pool_->run();
	acceptor_->listen();
}

void Server::handleNewConnection(int conn_fd, const InetAddr& addr)
{
	main_reactor_->assertInLoopThread();

	LOG_TRACE << "New connection from " << addr.toFormattedString();

	EventLoop* io_loop = sub_reactor_pool_->nextLoop();
	seq_.fetch_add(1, std::memory_order_acq_rel);

	std::shared_ptr<Connection> conn =
		std::make_shared<Connection>(conn_fd, io_loop, addr, seq_);

	conn->setConnectCallback(connect_callback_);
	conn->setMessageCallback(message_callback_);
	conn->setWriteCompleteCallback(write_complete_callback_);
	conn->setHighWaterMarkCallback(high_water_mark_callback_, high_water_mark_);
	conn->setCloseCallback(
		std::bind(&Server::handleClose, this, std::placeholders::_1));

	conn_map_[seq_] = conn;

	io_loop->runInLoop(std::bind(&Connection::connEstablish, conn));
}

void Server::handleClose(const std::shared_ptr<Connection>& conn)
{
	main_reactor_->runInLoop(std::bind(&Server::handleCloseInLoop, this, conn));
}

void Server::handleCloseInLoop(const std::shared_ptr<Connection>& conn)
{
	main_reactor_->assertInLoopThread();

	decltype(conn_map_)::iterator iter = conn_map_.find(conn->id());
	assert(iter != conn_map_.end());
	conn_map_.erase(iter);

	conn->loop()->queueInLoop(std::bind(&Connection::connDestroy, conn));
}