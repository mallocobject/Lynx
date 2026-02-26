#include "lynx/sql/session.hpp"
#include "lynx/tcp/inet_addr.hpp"
#include <format>
#include <mysql_driver.h>

using namespace lynx;
using namespace lynx::sql;

Session::Session(const std::string& ip, uint16_t port, const std::string& user,
				 const std::string& pass)
{
	driver_ = ::sql::mysql::get_driver_instance();
	std::string server_addr = std::format("tcp://{}:{}", ip, port);
	conn_.reset(driver_->connect(server_addr, user, pass));
}

Session::Session(const tcp::InetAddr& addr, const std::string& user,
				 const std::string& pass)
{
	driver_ = ::sql::mysql::get_driver_instance();
	std::string server_addr = "tcp://" + addr.toFormattedString();
	conn_.reset(driver_->connect(server_addr, user, pass));
}

Session::~Session()
{
}