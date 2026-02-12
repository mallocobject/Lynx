#include "lynx/logger/logger.h"
#include "lynx/tcp/buffer.h"
#include "lynx/tcp/event_loop.h"
#include "lynx/tcp/inet_addr.hpp"
#include "lynx/tcp/tcp_connection.h"
#include "lynx/tcp/tcp_server.h"
#include <memory>
#include <thread>

using namespace lynx;

int main()
{
	Logger::initAsyncLogging("logs/", "test");

	EventLoop loop;
	InetAddr addr(8080);
	TcpServer server(&loop, addr, "Lynx-Server",
					 std::thread::hardware_concurrency() - 2);

	server.setConnectionCallback(
		[](const std::shared_ptr<TcpConnection>& conn) {

		});

	server.setMessageCallback(
		[](const std::shared_ptr<TcpConnection>& conn, Buffer* buf)
		{
			std::string message = buf->retrieveString(buf->readableBytes());
			LOG_INFO << "-> " << message;
			conn->send(message);
		});

	loop.runEvery(2,
				  [&server]() {
					  LOG_WARN << "the number of connection is "
							   << server.connectionNum();
				  });

	server.run();
	loop.run();

	Logger::shutdownAsyncLogging();
}