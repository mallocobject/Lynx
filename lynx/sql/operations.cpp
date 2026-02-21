#include "lynx/sql/operations.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/sql/row_result.hpp"
#include "lynx/sql/types.hpp"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <format>
#include <iterator>
#include <memory>
#include <variant>

using namespace lynx;

CreateTableOp::CreateTableOp(sql::Connection* conn, const std::string& name,
							 bool if_not_exists)
	: conn_(conn), first_col_(true)
{
	if (if_not_exists)
	{
		sql_ = std::format("CREATE TABLE IF NOT EXISTS {} (", name);
	}
	else
	{
		sql_ = std::format("CREATE TABLE {} (", name);
	}
}

CreateTableOp& CreateTableOp::addColumn(const std::string& name,
										std::string_view type, int size)
{
	auto out = std::back_inserter(sql_);
	if (!first_col_)
	{
		std::format_to(out, ", ");
	}
	std::format_to(out, "{} {}", name, type);
	if (type == Type::STRING && size > 0)
	{
		std::format_to(out, "({})", size);
	}
	first_col_ = false;
	return *this;
}

CreateTableOp& CreateTableOp::primaryKey(const std::string& col_name)
{
	pks_.push_back(col_name);
	return *this;
}
void CreateTableOp::execute()
{
	auto out = std::back_inserter(sql_);
	if (!pks_.empty())
	{
		std::format_to(out, ", PRIMARY KEY ({}", pks_[0]);
		for (size_t i = 1; i < pks_.size(); i++)
		{
			std::format_to(out, ", {}", pks_[i]);
		}

		std::format_to(out, ")");
	}
	std::format_to(out, ")");
	LOG_INFO << "sql: " << sql_;
	std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
	stmt->execute(sql_);
}

RowResult SelectOp::execute()
{
	std::string result;
	auto out = std::back_inserter(result);
	std::format_to(out, "SELECT ");
	if (columns_.empty())
	{
		std::format_to(out, "*");
	}
	else
	{
		std::format_to(out, "{}", columns_[0]);
		for (size_t i = 1; i < columns_.size(); i++)
		{
			std::format_to(out, ", {}", columns_[i]);
		}
	}

	std::format_to(out, " FROM {}", table_name_);

	if (!where_clause_.empty())
	{
		std::format_to(out, " WHERE {}", where_clause_);
	}

	LOG_INFO << "sql: " << result;
	std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
	return RowResult(stmt->executeQuery(result));
}

namespace
{
struct VariantBinder
{
	sql::PreparedStatement* pstmt;
	unsigned int index;
	void operator()(const std::string& v)
	{
		pstmt->setString(index, v);
	}
	void operator()(int v)
	{
		pstmt->setInt(index, v);
	}
	void operator()(int64_t v)
	{
		pstmt->setInt64(index, v);
	}
	void operator()(double v)
	{
		pstmt->setDouble(index, v);
	}
	void operator()(bool v)
	{
		pstmt->setBoolean(index, v);
	}
};
} // namespace

void UpdateOp::execute()
{
	if (updates_.empty())
	{
		return;
	}

	std::string result;
	auto out = std::back_inserter(result);

	std::format_to(out, "UPDATE {} SET ", table_name_);
	bool first = true;
	for (const auto& [col, val] : updates_)
	{
		if (!first)
		{
			std::format_to(out, ", ");
		}
		std::format_to(out, "{} = ?", col);
		first = false;
	}

	if (!where_clause_.empty())
	{
		std::format_to(out, " WHERE {}", where_clause_);
	}

	LOG_INFO << "sql: " << result;
	std::unique_ptr<sql::PreparedStatement> pstmt(
		conn_->prepareStatement(result));

	unsigned int idx = 1;
	for (const auto& [col, val] : updates_)
	{
		// for variant
		std::visit(VariantBinder{pstmt.get(), idx++}, val);
	}
	pstmt->executeUpdate();
}

void RemoveOp::execute()
{
	std::string result;
	auto out = std::back_inserter(result);

	std::format_to(out, "DELETE FROM {}", table_name_);
	if (!where_clause_.empty())
	{
		std::format_to(out, " WHERE {}", where_clause_);
	}

	LOG_INFO << "sql: " << result;
	std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
	stmt->executeUpdate(result);
}