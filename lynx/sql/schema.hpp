#ifndef LYNX_SCHEMA_HPP
#define LYNX_SCHEMA_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/sql/operations.hpp"
#include "lynx/sql/table.hpp"
#include <cppconn/connection.h>
namespace lynx
{
class Schema : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string name_;

  public:
	Schema(sql::Connection* conn, const std::string& name)
		: conn_(conn), name_(name)
	{
		conn_->setSchema(name);
	}

	Table table(const std::string& table_name)
	{
		return Table(conn_, table_name);
	}

	CreateTableOp createTable(const std::string& table_name, bool if_not_exists)
	{
		return CreateTableOp(conn_, table_name, if_not_exists);
	}
};
} // namespace lynx

#endif