#include "lynx/include/buffer.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

// 设置文件描述符为非阻塞
void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 忽略 SIGPIPE 信号，防止对端关闭后发送数据导致进程退出
void ignoreSigpipe()
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, nullptr);
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
		return 1;
	}

	ignoreSigpipe();

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setNonBlocking(sockfd);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(std::stoi(argv[2]));
	inet_pton(AF_INET, argv[1], &addr.sin_addr);

	// 非阻塞连接
	int ret = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
	if (ret == -1 && errno != EINPROGRESS)
	{
		perror("connect");
		return 1;
	}

	int epfd = epoll_create1(0);
	epoll_event events[10];

	// 监听：1. Socket 可读  2. 标准输入可读
	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

	lynx::Buffer inputBuffer;  // 接收网络数据
	lynx::Buffer outputBuffer; // 发送网络数据

	bool running = true;
	std::cout << "Connected to server. Type something..." << std::endl;

	while (running)
	{
		int nfds = epoll_wait(epfd, events, 10, -1);
		for (int i = 0; i < nfds; ++i)
		{
			int fd = events[i].data.fd;

			// 情况 1: Socket 可读 (服务器回显)
			if (fd == sockfd && (events[i].events & EPOLLIN))
			{
				int savedErrno = 0;
				ssize_t n = inputBuffer.readFd(sockfd, &savedErrno);
				if (n > 0)
				{
					while (inputBuffer.readableBytes() >= sizeof(int32_t))
					{
						int32_t len = inputBuffer.peekInt32();
						if (inputBuffer.readableBytes() >=
							static_cast<size_t>(len + 4))
						{
							inputBuffer.retrieve(sizeof(int32_t)); // 跳过长度头
							std::string msg =
								inputBuffer.retrieveString(len); // 读取内容
							std::cout << "\rServer msg: " << msg << std::endl
									  << "> " << std::flush;
						}
						else
						{
							break;
						}
					}
				}

				else if (n == 0)
				{
					std::cout << "\nServer closed connection." << std::endl;
					running = false;
				}
			}

			// 情况 2: 标准输入可读 (用户输入)
			else if (fd == STDIN_FILENO)
			{
				std::string line;
				if (!std::getline(std::cin, line) || line == "[^")
				{ // 处理 Ctrl+D
					std::cout << "Exiting..." << std::endl;
					shutdown(sockfd, SHUT_WR);
					// 移除键盘监听，继续处理剩余的网络接收
					epoll_ctl(epfd, EPOLL_CTL_DEL, STDIN_FILENO, nullptr);
					continue;
				}

				if (line.empty())
				{
					continue;
				}

				// --- 关键：封包过程 ---
				// 1. 写入 4 字节大端长度
				int32_t len = static_cast<int32_t>(line.size());
				outputBuffer.appendInt32(len);
				// 2. 写入内容
				outputBuffer.append(line.data(), line.size());

				// 尝试直接发送
				ssize_t n = write(sockfd, outputBuffer.peek(),
								  outputBuffer.readableBytes());
				if (n > 0)
					outputBuffer.retrieve(n);

				// 如果没发完，注册可写事件
				if (outputBuffer.readableBytes() > 0)
				{
					ev.events = EPOLLIN | EPOLLOUT;
					ev.data.fd = sockfd;
					epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
				}
				std::cout << "> " << std::flush;
			}

			// 情况 3: Socket 可写 (发送之前积压的数据)
			else if (fd == sockfd && (events[i].events & EPOLLOUT))
			{
				ssize_t n = write(sockfd, outputBuffer.peek(),
								  outputBuffer.readableBytes());
				if (n > 0)
				{
					outputBuffer.retrieve(n);
					if (outputBuffer.readableBytes() == 0)
					{
						// 发完了，关掉可写监听
						ev.events = EPOLLIN;
						ev.data.fd = sockfd;
						epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
					}
				}
			}
		}
	}

	close(sockfd);
	return 0;
}