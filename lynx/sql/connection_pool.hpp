#ifndef LYNX_CONNECTION_POOL_HPP
#define LYNX_CONNECTION_POOL_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
namespace lynx
{
class Session;
class InetAddr;
class ConnectionPool : public noncopyable
{
  private:
	std::queue<Session*> sesses_;
	std::string ip_;
	uint16_t port_;
	std::string user_;
	std::string pass_;
	std::string db_name_;

	std::mutex mtx_;
	std::condition_variable cv_;

	std::atomic<bool> stop_;

	uint32_t cur_conn_num_;
	uint32_t min_conn_num_;
	uint32_t max_conn_num_;

  public:
	ConnectionPool(const std::string& ip, uint16_t port,
				   const std::string& user, const std::string& pass,
				   uint32_t min_conn_num, uint32_t max_conn_num);

	ConnectionPool(const InetAddr& addr, const std::string& user,
				   const std::string& pass, uint32_t min_conn_num,
				   uint32_t max_conn_num);

	~ConnectionPool();

	std::shared_ptr<Session> connection();

  private:
	Session* createSession();

	void returnConnection(Session* sess);

	void close();
};
} // namespace lynx

#endif