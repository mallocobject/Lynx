#ifndef LYNX_FORMATTER_HPP
#define LYNX_FORMATTER_HPP

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/logger/context.hpp"
#include "lynx/logger/flag.hpp"
#include <cstring>
#include <string>

#if __cplusplus >= 202002L
#include <format>
#endif

namespace lynx
{

class Formatter
{
  public:
	DISABLE_COPY(Formatter)

	explicit Formatter(Flags flags = Flags::kStdFlags) : flags_(flags)
	{
	}

	/**
	 * 将Context格式化为日志字符串（使用std::format_to直接写入）
	 * @param ctx 日志上下文
	 * @return 格式化后的日志字符串
	 */
	std::string format(const Context& ctx) const
	{
		return formatWithFormatTo(ctx);
	}

  private:
	Flags flags_;

	// C++20 std::format_to 版本（最优性能）- 直接写入缓冲区
	std::string formatWithFormatTo(const Context& ctx) const
	{
		std::string result;
		auto out = std::back_inserter(result);

		// 日期
		if (flags_ & Flags::kDate)
		{
			std::format_to(
				out, "{} ",
				TimeStamp(ctx.timestamp).toFormattedString(false, true));
		}

		// 时间
		if (flags_ & Flags::kTime)
		{
			std::format_to(
				out, "{} ",
				TimeStamp(ctx.timestamp).toFormattedString(true, false));
		}

		// 线程ID
		if (flags_ & Flags::kThreadId)
		{
			std::format_to(out, "[{}] ", ctx.tid);
		}

		// 日志级别
		std::format_to(out, "{} ", getLogLevelString(ctx.level));

		// 短文件名
		if (flags_ & Flags::kShortName)
		{
			std::format_to(out, "{}:{} ", ctx.data.short_filename,
						   ctx.data.line);
		}

		// 完整文件名
		if (flags_ & Flags::kFullName)
		{
			std::format_to(out, "{}:{} ", ctx.data.full_filename,
						   ctx.data.line);
		}

		// 函数名
		if (flags_ & Flags::kFuncName)
		{
			std::format_to(out, "{}() ", ctx.data.func_name);
		}

		// 日志内容
		std::format_to(out, "{}", ctx.text);

		return result;
	}

	static const char* getLogLevelString(int level)
	{
		switch (level)
		{
		case 0:
			return "[TRACE]";
		case 1:
			return "[DEBUG]";
		case 2:
			return "[INFO]";
		case 3:
			return "[WARN]";
		case 4:
			return "[ERROR]";
		case 5:
			return "[FATAL]";
		default:
			return "[UNKNOWN]";
		}
	}
};

} // namespace lynx

#endif // LYNX_FORMATTER_HPP
