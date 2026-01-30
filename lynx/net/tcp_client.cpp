#include "lynx/net/tcp_client.h"
#include "lynx/base/logger.hpp"
#include "lynx/net/connector.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_connection.h"
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>

namespace lynx
{
TcpClient::TcpClient(EventLoop* loop, const char* serv_ip, uint16_t serv_port,
					 const std::string name)
	: loop_(loop), name_(name),
	  connector_(std::make_shared<Connector>(loop, serv_ip, serv_port)),
	  serv_port_(serv_port), next_conn_id_(1)
{
	strcpy(serv_ip_, serv_ip);

	connector_->setNewConnectionCallback(std::bind(
		&TcpClient::handleNewConnection, this, std::placeholders::_1));
	LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector " << this;
}

TcpClient::~TcpClient()
{
	LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector " << this;
	if (conn_)
	{
		assert(loop_ == conn_->loop());
		conn_->forceClose();
	}
	else
	{
		connector_->stop();
	}
}

void TcpClient::connect()
{
	LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
			 << serv_ip_ << ':' << serv_port_;
	connector_->startup();
}

void TcpClient::disconnect()
{
	if (conn_)
	{
		conn_->shutdown();
	}
}

void TcpClient::stop()
{
	connector_->stop();
}

void TcpClient::handleNewConnection(int fd)
{
	loop_->assertInLocalThread();
	char buf[64];
	snprintf(buf, sizeof(buf), "-%s:%d#%d", serv_ip_, serv_port_,
			 next_conn_id_++);
	std::string conn_name = name_ + buf;

	conn_ = std::make_shared<TcpConnection>(fd, loop_, conn_name, "0.0.0.0", 0,
											serv_ip_, serv_port_);
	conn_->setConnectCallback(connection_callback_);
	conn_->setMessageCallback(message_callback_);
	conn_->setWriteCompleteCallback(write_complete_callback_);
	conn_->setCloseCallback(
		std::bind(&TcpClient::handleClose, this, std::placeholders::_1));

	loop_->runInLocalThread(std::bind(&TcpConnection::establish, conn_));
}

void TcpClient::handleClose(const std::shared_ptr<TcpConnection>& conn)
{
	loop_->assertInLocalThread();
	assert(loop_ == conn_->loop());
	conn_.reset();
	loop_->queueInLocalThread(std::bind(&TcpConnection::destroy, conn));
}
} // namespace lynx