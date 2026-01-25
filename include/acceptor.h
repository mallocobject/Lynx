#ifndef LYNX_ACCEPTOR_H
#define LYNX_ACCEPTOR_H

#include "lynx/include/common.h"
#include <functional>
#include <memory>
namespace lynx
{
class EventLoop;
class Channel;
class Acceptor
{
  private:
	EventLoop* loop_;
	std::unique_ptr<Channel> ch_;
	int fd_;
	bool listening_;
	std::function<void(int fd, const char* ip, uint16_t port)>
		new_connection_callback_;
	int idle_fd_; // 预留空闲fd

  public:
	DISABLE_COPY(Acceptor)

	Acceptor(EventLoop* loop, const char* ip, uint16_t port);
	~Acceptor();

	void setNewConnectionCallback(
		const std::function<void(int fd, const char* ip, uint16_t port)>& cb)
	{
		new_connection_callback_ = cb;
	}

	bool listening() const
	{
		return listening_;
	}

	void listen();

  private:
	void handleRead();
};
} // namespace lynx

#endif