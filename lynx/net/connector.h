#ifndef LYNX_CONNECTOR_H
#define LYNX_CONNECTOR_H

#include "lynx/base/common.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <utility>
namespace lynx
{
class EventLoop;
class Channel;
class Connector : public std::enable_shared_from_this<Connector>
{
  private:
	// int fd_;
	std::unique_ptr<Channel> ch_;
	char serv_ip_[INET_ADDRSTRLEN];
	uint16_t serv_port_;
	EventLoop* loop_;
	bool connect_;
	int retry_delay_ms_;

	enum State
	{
		Connecting,
		Connected,
		Disconnected
	} state_;

	std::function<void(int)> new_connection_callback;

  public:
	DISABLE_COPY(Connector)

	Connector(EventLoop* loop, const char* serv_ip, uint16_t serv_port);
	~Connector();

	void setNewConnectionCallback(std::function<void(int)> cb)
	{
		new_connection_callback = std::move(cb);
	}

	void startup();
	void restart(); // must be called in local loop thread
	void stop();

  private:
	void startInLocalLoop();
	void stopInLocalLoop();

	void connect();
	void connecting();

	void handleWrite();
	void handleError();

	void retry();

	void removeAndResetChannel();
	void resetChannel();

	bool isSelfConnect() const;
};
} // namespace lynx

#endif