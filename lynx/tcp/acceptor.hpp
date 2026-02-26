#ifndef LYNX_TCP_ACCEPTOR_HPP
#define LYNX_TCP_ACCEPTOR_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <functional>
#include <memory>
#include <utility>
namespace lynx
{
namespace tcp
{
class EventLoop;
class Channel;
class Acceptor : base::noncopyable
{
  private:
	EventLoop* loop_;
	std::unique_ptr<Channel> ch_;
	InetAddr addr_;
	bool listening_;
	std::function<void(int, const InetAddr&)> new_connection_callback_;
	int idle_fd_;

  public:
	Acceptor(EventLoop* loop, const InetAddr& local_addr);
	~Acceptor();

	void setNewConnectionCallback(std::function<void(int, const InetAddr&)> cb)
	{
		new_connection_callback_ = std::move(cb);
	}

	bool listening() const
	{
		return listening_;
	}

	void listen();

	int fd() const;

	EventLoop* loop() const
	{
		return loop_;
	}

  private:
	void handleRead();
};
} // namespace tcp
} // namespace lynx

#endif