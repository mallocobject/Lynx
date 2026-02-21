#ifndef LYNX_OPERATION_HPP
#define LYNX_OPERATION_HPP

#include "logger.hpp"
#include "noncopyable.hpp"
#include "row_result.hpp"
#include "types.hpp"
#include <cmath>
#include <concepts>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <format>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace lynx
{
class CreateTableOp : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string sql_;
	std::vector<std::string> pks_; // primary key(s)
	bool first_col_;

  public:
	CreateTableOp(sql::Connection* conn, const std::string& name,
				  bool if_not_exists);
	CreateTableOp& addColumn(const std::string& name, std::string_view type,
							 int size = 0);
	CreateTableOp& primaryKey(const std::string& col_name);
	void execute();
};

class InsertOp : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string table_name_;
	std::vector<std::string> columns_;

  public:
	InsertOp(sql::Connection* conn, const std::string& name,
			 std::vector<std::string> cols)
		: conn_(conn), table_name_(name), columns_(std::move(cols))
	{
	}

	template <SqlParamType... Args> InsertOp& values(Args&&... args)
	{
		if (sizeof...(args) != columns_.size())
		{
			LOG_ERROR << "Column count mismatch";
			return *this;
		}
		if (sizeof...(args) == 0)
		{
			return *this;
		}

		std::string result;
		auto out = std::back_inserter(result);
		// ss << "INSERT INTO " << table_name_ << " (";
		std::format_to(out, "INSERT INTO {} ({}", table_name_, columns_[0]);

		for (size_t i = 1; i < columns_.size(); i++)
		{
			// ss << (i > 0 ? ", " : "") << columns_[i];

			std::format_to(out, ", {}", columns_[i]);
		}

		// ss << ") VALUES (";
		std::format_to(out, ") VALUES (?");

		size_t n = sizeof...(args);
		for (size_t i = 1; i < n; i++)
		{
			// ss << (i > 0 ? ", ?" : "?");
			std::format_to(out, ", ?");
		}
		// ss << ")";

		std::format_to(out, ")");

		LOG_INFO << "sql: " << result;
		std::unique_ptr<sql::PreparedStatement> pstmt(
			conn_->prepareStatement(result));

		unsigned int idx = 1;
		(internal::bind_impl(pstmt.get(), idx++, std::forward<Args>(args)),
		 ...);

		pstmt->executeUpdate();
		return *this;
	}

	void execute()
	{
	}
};

class SelectOp : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string table_name_;
	std::vector<std::string> columns_;
	std::string where_clause_;

  public:
	SelectOp(sql::Connection* conn, const std::string& name,
			 std::vector<std::string> cols)
		: conn_(conn), table_name_(name), columns_(std::move(cols))

	{
	}

	SelectOp& where(const std::string& condition)
	{
		where_clause_ = condition;
		return *this;
	}

	RowResult execute();
};

class UpdateOp : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string table_name_;
	std::string where_clause_;
	std::map<std::string, DbValue> updates_;

  public:
	UpdateOp(sql::Connection* conn, const std::string& name)
		: conn_(conn), table_name_(name)
	{
	}

	template <SqlParamType T> UpdateOp& set(const std::string& col, T val)
	{
		if constexpr (std::convertible_to<T, std::string>)
		{
			// updates_[std::string(col)] = std::string(val);
			updates_.emplace(std::string(col), std::string(val));
		}
		else
		{
			// updates_[std::string(col)] = val;
			updates_.emplace(std::string(col), std::move(val));
		}
		return *this;
	}

	UpdateOp& where(const std::string& condition)
	{
		where_clause_ = condition;
		return *this;
	}

	void execute();
};

class RemoveOp : public noncopyable
{
  private:
	sql::Connection* conn_;
	std::string table_name_;
	std::string where_clause_;

  public:
	RemoveOp(sql::Connection* conn, const std::string& name)
		: conn_(conn), table_name_(name)
	{
	}

	RemoveOp& where(const std::string& condition)
	{
		where_clause_ = condition;
		return *this;
	}

	void execute();
};
} // namespace lynx

#endif