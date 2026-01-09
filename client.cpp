#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

void writeBytes(int fd, const char* buf, size_t n)
{
	size_t bytes_left = n;
	while (bytes_left > 0)
	{
		ssize_t bytes_written = ::write(fd, buf + n - bytes_left, bytes_left);
		if (bytes_written > 0)
		{
			bytes_left -= bytes_written;
		}
		else if (bytes_written < 0 && errno != EINTR)
		{
			std::perror("writeBytes");
			exit(1);
		}
	}
}

void readBytes(int fd, char* buf, size_t* size)
{
	ssize_t bytes_read = ::read(fd, buf, *size);
	*size = bytes_read;

	if (bytes_read == 0)
	{
		std::cout << "the other side closed" << std::endl;
	}
	else if (bytes_read < 0 && errno != EINTR)
	{
		std::perror("readBytes");
		exit(1);
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "[program] [ip] [port]" << std::endl;
		exit(1);
	}
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	::bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(::atoi(argv[2]));
	::inet_pton(AF_INET, argv[1], &addr.sin_addr);

	int ret = ::connect(sockfd, (sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		std::perror(nullptr);
		exit(1);
	}

	char buf[1024];
	::bzero(buf, sizeof(buf));
	size_t size = sizeof(buf) - 1;

	int epfd = ::epoll_create1(0);
	epoll_event ev{EPOLLIN, {.fd = sockfd}};
	epoll_event events[2];
	::epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	ev = {EPOLLIN, {.fd = STDIN_FILENO}};
	::epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

	while (true)
	{
		int nfds = ::epoll_wait(epfd, events, 2, -1);
		if (nfds == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				perror("epoll");
				break;
			}
		}
		for (int i = 0; i < nfds; i++)
		{
			if (events[i].data.fd == sockfd)
			{
				std::cout << events[i].events << std::endl;
				if (events[i].events & (EPOLLERR))
				{
					std::cout << "error occurred on server" << std::endl;
					goto flag_exit;
				}
				if (events[i].events & EPOLLIN)
				{
					::bzero(buf, sizeof(buf));
					size = sizeof(buf) - 1;
					readBytes(sockfd, buf, &size);
					if (size == 0)
					{
						goto flag_exit;
					}
					buf[size] = 0;
					::fputs("echo: ", stdout);
					::fputs(buf, stdout);
					::fputs("([^ to exit)\n", stdout);
				}
			}
			else if (events[i].data.fd == STDIN_FILENO)
			{
				if (events[i].events & EPOLLIN)
				{
					::bzero(buf, sizeof(buf));
					size = sizeof(buf);
					std::cout << std::flush;
					if (::fgets(buf, size, stdin) == NULL)
					{
						std::cout << "\nEOF received, exiting..." << std::endl;
						// goto flag_exit;
						::shutdown(sockfd, SHUT_WR);
						continue;
					}
					else
					{
						buf[strcspn(buf, "\n")] = 0;
						writeBytes(sockfd, buf, strlen(buf));
					}
				}
			}
		}
	}

flag_exit:

	::close(sockfd);
}