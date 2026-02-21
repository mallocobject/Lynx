#ifndef LYNX_TABLE_HPP
#define LYNX_TABLE_HPP
#include "lynx/base/noncopyable.hpp"
#include "lynx/sql/operations.hpp"
#include <cppconn/connection.h>
#include <string>
#include <vector>
namespace lynx
{
class Table : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string name_;

  public:
	Table(sql::Connection* conn, const std::string& name)
		: conn_(conn), name_(name)
	{
	}

	template <typename... Args> InsertOp insert(Args... cols)
	{
		std::vector<std::string> col_vec{static_cast<std::string>(cols)...};
		return InsertOp(conn_, name_, col_vec);
	}

	template <typename... Args> SelectOp select(Args... cols)
	{
		std::vector<std::string> col_vec{static_cast<std::string>(cols)...};
		return SelectOp(conn_, name_, col_vec);
	}

	UpdateOp update()
	{
		return UpdateOp(conn_, name_);
	}

	RemoveOp remove()
	{
		return RemoveOp(conn_, name_);
	}
};
} // namespace lynx

#endif