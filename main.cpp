// #define NODEBUG

#include "lynx/include/channel.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include "lynx/include/tcp_connection.h"
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	// lynx::LOG_INFO() << "hello world";
	// lynx::LOG_WARN() << "hello world";
	// lynx::LOG_ERROR() << "hello world";
	// lynx::LOG_FATAL() << "hello world";
	// lynx::LOG_DEBUG() << "hello world";

	// todo
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	lynx::Channel listen_ch(listen_fd, nullptr);
	listen_ch.setReuseAddr(true);

	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8234);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	int ret = bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
	listen(listen_fd, SOMAXCONN);
	sockaddr_in clnt_addr;
	::bzero(&clnt_addr, sizeof(clnt_addr));
	socklen_t addr_len = sizeof(clnt_addr);
	int conn_fd = ::accept4(listen_fd, (sockaddr*)&clnt_addr, &addr_len,
							SOCK_NONBLOCK | SOCK_CLOEXEC);

	char ip_v4[INET_ADDRSTRLEN];
	bzero(ip_v4, sizeof(ip_v4));

	lynx::EventLoop loop;
	auto conn = std::make_shared<lynx::TcpConnection>(
		conn_fd, &loop,
		inet_ntop(AF_INET, &clnt_addr.sin_addr, ip_v4, sizeof(ip_v4)),
		ntohs(clnt_addr.sin_port));

	conn->setMessageCallback(
		[](const std::shared_ptr<lynx::TcpConnection>&) {

		});
	conn->setWriteCompleteCallback(
		[](const std::shared_ptr<lynx::TcpConnection>&) {

		});
	conn->setConnectCallback(
		[](const std::shared_ptr<lynx::TcpConnection>&) {

		});

	conn->setCloseCallback(
		[](const std::shared_ptr<lynx::TcpConnection>&) {

		});

	conn->establish();

	loop.run();
}