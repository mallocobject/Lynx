#include <lynx/lynx.hpp>

using namespace lynx;

int main(int argc, char* argv[])
{
	// 初始化日志
	Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", argv[0]);

	// 创建 TCP 服务器
	EventLoop loop;

	TcpServer server(&loop, "0.0.0.0", 9999, "EchoServer", 8);

	// 设置连接回调
	server.setConnectionCallback(
		[](const std::shared_ptr<TcpConnection>& conn)
		{
			if (conn->connected())
			{
				LOG_INFO << "Client connected: "
						 << conn->addr().toFormattedString();
			}
			if (conn->disconnected())
			{
				LOG_INFO << "Client disconnected: "
						 << conn->addr().toFormattedString();
			}
		});

	// 设置消息回调（Echo 逻辑）
	server.setMessageCallback(
		[](const std::shared_ptr<TcpConnection>& conn, Buffer* buf)
		{
			std::string msg = buf->retrieveString(buf->readableBytes());
			conn->send(msg); // 原样返回
			LOG_INFO << "Echo: " << msg;
		});

	LOG_INFO << "Echo Server listening on 0.0.0.0:9999";
	server.run();
	loop.run();

	return 0;
}