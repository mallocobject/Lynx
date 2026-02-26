#ifndef LYNX_SQL_TYPES_HPP
#define LYNX_SQL_TYPES_HPP

#include <concepts>
#include <cppconn/prepared_statement.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
namespace lynx
{
namespace sql
{
template <typename T>
concept SqlParamType = std::integral<T> || std::floating_point<T> ||
					   std::convertible_to<T, std::string>;

struct Type
{
	static constexpr std::string_view INT = "INT";
	static constexpr std::string_view STRING = "VARCHAR";
	static constexpr std::string_view DOUBLE = "DOUBLE";
	static constexpr std::string_view BOOL = "TYNYINT";
};

using DbValue = std::variant<int, int64_t, double, bool, std::string>;

namespace internal
{
// 参数绑定辅助函数 (inline 防止多重定义)
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  const std::string& val)
{
	pstmt->setString(index, val);
}
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  const char* val)
{
	pstmt->setString(index, val);
}
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  int val)
{
	pstmt->setInt(index, val);
}
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  int64_t val)
{
	pstmt->setInt64(index, val);
}
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  double val)
{
	pstmt->setDouble(index, val);
}
inline void bind_impl(::sql::PreparedStatement* pstmt, unsigned int index,
					  bool val)
{
	pstmt->setBoolean(index, val);
}
} // namespace internal
} // namespace sql
} // namespace lynx

#endif