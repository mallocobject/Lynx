#ifndef LYNX_TCP_CONNECTION_H
#define LYNX_TCP_CONNECTION_H

#include "lynx/base/noncopyable.hpp"
#include <memory>
namespace lynx
{
class Channel;
class EventLoop;
class Buffer;
class TcpConnection : public noncopyable,
					  public std::enable_shared_from_this<TcpConnection>
{
  private:
	enum State
	{
		kDisconnected,
		kConnecting,
		kConnected,
		kDisconnecting
	} state_;
	// establish: Connecting -> Connected
	// shutdown: Connected -> Disconnecting
	// handleColse: Connected/Disconnecting -> Disconnected
	// destroy: [Connected/Disconnecting -> Disconnected]
};
} // namespace lynx

#endif