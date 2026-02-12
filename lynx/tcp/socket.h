#ifndef LYNX_SOCKET_H
#define LYNX_SOCKET_H

#include "lynx/tcp/inet_addr.hpp"
#include <cstdlib>
#include <sys/socket.h>

#define checkErrno(saved_errno)                                                \
	do                                                                         \
	{                                                                          \
		if ((saved_errno) != 0)                                                \
		{                                                                      \
			exit(EXIT_FAILURE);                                                \
		}                                                                      \
	} while (0)

namespace lynx
{
namespace Socket
{
// only for connecting, because it is deferred
int socketErrno(int fd);
int socket(int* saved_errno);

void setNonBlocking(int fd, bool on = true);
void setReuseAddr(int fd, bool on = true);
void setKeepAlive(int fd, bool on = true);
void setNoDelay(int fd, bool on = true);

bool bind(int fd, const InetAddr& local_addr, int* saved_errno);
bool listen(int fd, int* saved_errno, int backlog = SOMAXCONN);
int accept(int fd, InetAddr* peer_addr, int* saved_errno);
bool connect(int fd, const InetAddr& serv_addr, int* saved_errno);

void shutdown(int fd);
void close(int fd);

} // namespace Socket
} // namespace lynx

#endif