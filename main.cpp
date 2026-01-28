#include "lynx/include/buffer.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include "lynx/include/tcp_connection.h"
#include "lynx/include/tcp_server.h"
#include <cstdint>
#include <memory>
#include <thread>
#include <unistd.h>
int main()
{
	lynx::LOG_INFO << "main(): pid = " << std::this_thread::get_id();
	lynx::EventLoop loop;
	lynx::TcpServer server(&loop, "127.0.0.1", 8234, "Lynx");
	server.setMessageCallback(
		[](const std::shared_ptr<lynx::TcpConnection>& conn,
		   std::shared_ptr<lynx::Buffer> buf)
		{
			while (buf->readableBytes() >= sizeof(int32_t))
			{
				int32_t len = buf->peekInt32();
				if (buf->readableBytes() >=
					static_cast<size_t>(len + sizeof(int32_t)))
				{
					buf->retrieve(sizeof(int32_t));
					std::string message = buf->retrieveString(len);
					lynx::LOG_INFO << "-> " << message;
				}
				else
				{
					break;
				}
			}
		});

	server.startup();
}