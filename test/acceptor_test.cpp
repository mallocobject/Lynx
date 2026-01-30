#include "lynx/base/logger.hpp"
#include "lynx/net/acceptor.h"
#include "lynx/net/event_loop.h"
#include <cstdint>
#include <thread>
#include <unistd.h>
int main()
{
	lynx::LOG_INFO << "main(): pid = " << std::this_thread::get_id();
	lynx::EventLoop loop;
	lynx::Acceptor acceptor(&loop, "127.0.0.1", 8234);
	acceptor.setNewConnectionCallback(
		[](int conn_fd, const char* peer_ip, uint16_t peer_port)
		{
			lynx::LOG_INFO << "newConnection(): accepted a new connection from "
						   << peer_ip << ':' << peer_port;
			::write(conn_fd, "How are you?", 13);
			::close(conn_fd);
		});
	acceptor.listen();

	loop.run();
}