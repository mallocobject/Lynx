#include "lynx/logger/logger.h"
#include "lynx/net/buffer.h"
#include "lynx/net/channel.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_client.h"
#include "lynx/net/tcp_connection.h"
#include <memory>
#include <signal.h>
#include <sys/epoll.h>
#include <unistd.h>

static bool ignore_sig = []()
{
	signal(SIGPIPE, SIG_IGN);
	return true;
}();

int main()
{
	lynx::EventLoop loop;
	lynx::TcpClient client(&loop, "127.0.0.1", 8234, "Lynx-client");

	std::shared_ptr<lynx::TcpConnection> current_conn; // for keyboard

	client.setConnectionCallback(
		[&](const std::shared_ptr<lynx::TcpConnection>& conn)
		{
			if (conn->connected())
			{
				current_conn = conn;
				lynx::LOG_INFO << "connected to server";
				std::cout << "Type something...\n> " << std::flush;
			}
			else
			{
				current_conn.reset();
				lynx::LOG_INFO << "connection is down";
				loop.quit();
			}
		});

	client.setMessageCallback(
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

	lynx::Channel stdin_ch(STDIN_FILENO, &loop);
	stdin_ch.setReadCallback(
		[&]()
		{
			std::string line;
			if (!std::getline(std::cin, line) || line == "[^")
			{
				std::cout << "Exiting..." << std::endl;
				if (current_conn)
				{
					current_conn->shutdown(); // 半关闭网络连接 (SHUT_WR)
				}
				stdin_ch.disableAll(); // 停止监听键盘
				stdin_ch.remove();	   // 从 epoll 移除
				return;
			}

			if (line.empty())
			{
				std::cout << "> " << std::flush;
				return;
			}

			// --- 封包并发送 ---
			if (current_conn)
			{
				lynx::Buffer outBuf;
				int32_t len = static_cast<int32_t>(line.size());
				outBuf.appendInt32(len); // 1. 写入 4 字节大端长度
				outBuf.append(line.data(), len); // 2. 写入内容

				// 发送完整 Buffer 数据
				current_conn->send(
					outBuf.retrieveString(outBuf.readableBytes()));
				std::cout << "> " << std::flush;
			}
		});

	stdin_ch.enableIN();
	client.connect();

	loop.run();
}