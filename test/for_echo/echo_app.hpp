#include "lynx/lynx.hpp"
#include <functional>
#include <lynx/tcp_connection.hpp>

class EchoApp : public lynx::noncopyable
{
  private:
	using cc_type = lynx::CircularBuffer<lynx::Entry<lynx::TcpConnection>>;

	lynx::TcpServer server_;
	cc_type conn_buckets_;

  public:
	EchoApp(lynx::EventLoop* loop, const std::string& ip, uint16_t port,
			const std::string& name,
			size_t sub_reactor_num = std::thread::hardware_concurrency())
		: server_(loop, ip, port, name, sub_reactor_num)
	{
		server_.setConnectionCallback(
			std::bind(&EchoApp::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(std::bind(&EchoApp::onMessage, this,
											 std::placeholders::_1,
											 std::placeholders::_2));
		loop->runEvery(1.0, std::bind(&EchoApp::onTimer, this));
	}

	void run()
	{
		server_.run();
	}

	size_t connectionNum() const
	{
		return server_.connectionNum();
	}

	size_t connectionNumInBuckets() const
	{
		return conn_buckets_.size();
	}

  private:
	void onTimer()
	{
		conn_buckets_.push_back(cc_type::Bucket());
	}

	virtual void onConnection(const std::shared_ptr<lynx::TcpConnection>& conn)
	{
		if (conn->connected())
		{
			LOG_INFO << "New connection from "
					 << conn->addr().toFormattedString();
			std::shared_ptr<lynx::Entry<lynx::TcpConnection>> entry =
				std::make_shared<lynx::Entry<lynx::TcpConnection>>(
					conn,
					std::bind(&lynx::TcpConnection::shutdown, conn.get()));
			conn_buckets_.push_back(entry);
			std::weak_ptr<lynx::Entry<lynx::TcpConnection>> weak_entry = entry;
			conn->setContext(weak_entry);
		}
		else if (conn->disconnected())
		{
			LOG_INFO << "Connection closed on "
					 << conn->addr().toFormattedString();
			assert(conn->context().has_value());
			std::weak_ptr<lynx::Entry<lynx::TcpConnection>> weak_entry =
				std::any_cast<std::weak_ptr<lynx::Entry<lynx::TcpConnection>>>(
					conn->context());
			LOG_DEBUG << "Entry use_count = " << weak_entry.use_count();
		}
	}

	virtual void onMessage(const std::shared_ptr<lynx::TcpConnection>& conn,
						   lynx::Buffer* buf)
	{
		std::string message = buf->retrieveString(buf->readableBytes());
		LOG_INFO << "-> " << message;
		conn->send(message);

		assert(conn->context().has_value());
		std::weak_ptr<lynx::Entry<lynx::TcpConnection>> weak_entry =
			std::any_cast<std::weak_ptr<lynx::Entry<lynx::TcpConnection>>>(
				conn->context());
		std::shared_ptr<lynx::Entry<lynx::TcpConnection>> entry =
			weak_entry.lock();
		if (entry)
		{
			conn_buckets_.push_back(entry);
		}
	}
};