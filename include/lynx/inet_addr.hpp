#ifndef LYNX_INET_ADDR_HPP
#define LYNX_INET_ADDR_HPP

#include "logger.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <format>
#include <netinet/in.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
namespace lynx
{
class InetAddr
{
  private:
	struct sockaddr_in addr_;

  public:
	InetAddr()
	{
		::bzero(&addr_, sizeof(addr_));
	}

	explicit InetAddr(uint16_t port, bool loopback_only = false)
	{
		::bzero(&addr_, sizeof(addr_));
		addr_.sin_family = AF_INET;
		addr_.sin_port = ::htons(port);
		in_addr_t ip = loopback_only ? INADDR_LOOPBACK : INADDR_ANY;
		addr_.sin_addr.s_addr = ::htonl(ip);
	}

	InetAddr(const std::string& ip, uint16_t port)
	{
		::bzero(&addr_, sizeof(addr_));
		addr_.sin_family = AF_INET;
		addr_.sin_port = ::htons(port);
		if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr) != 1)
		{
			LOG_FATAL << "inet_pton failed: " << ::strerror(errno);
		}
	}

	explicit InetAddr(const sockaddr_in& addr) : addr_(addr)
	{
	}

	const struct sockaddr* sockaddr() const
	{
		return reinterpret_cast<const struct sockaddr*>(&addr_);
	}

	struct sockaddr* sockaddr()
	{
		return reinterpret_cast<struct sockaddr*>(&addr_);
	}

	void setSockaddr(const sockaddr_in& addr)
	{
		addr_ = addr;
	}

	std::string ip() const
	{
		char buf[INET_ADDRSTRLEN];
		::bzero(buf, sizeof(buf));
		if (::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == nullptr)
		{
			LOG_ERROR << "inet_ntop failed: " << ::strerror(errno);
		}

		return std::string(buf);
	}

	uint16_t port() const
	{
		return ::ntohs(addr_.sin_port);
	}

	std::string toFormattedString() const
	{
		return std::format("{}:{}", ip(), port());
	}
};
} // namespace lynx

#endif