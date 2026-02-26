#ifndef LYNX_SQL_SESSION_HPP
#define LYNX_SQL_SESSION_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/sql/schema.hpp"
#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cstdint>
#include <memory>
namespace lynx
{
namespace tcp
{
class InetAddr;
}
namespace sql
{
class Session : public base::noncopyable
{
  private:
	::sql::Driver* driver_;
	std::unique_ptr<::sql::Connection> conn_;

  public:
	Session(const std::string& ip, uint16_t port, const std::string& user,
			const std::string& pass);

	Session(const tcp::InetAddr& addr, const std::string& user,
			const std::string& pass);
	~Session();

	Schema schema(const std::string& db_name)
	{
		return Schema(conn_.get(), db_name);
	}

	::sql::Connection* nativeConnection()
	{
		return conn_.get();
	}
};
} // namespace sql
} // namespace lynx

#endif