#ifndef LYNX_PUBLIC_LOGGER_HPP
#define LYNX_PUBLIC_LOGGER_HPP

#include "current_thread.hpp"
#include "noncopyable.hpp"
#include "time_stamp.hpp"
#include <atomic>
#include <cassert>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>
#include <type_traits>
namespace lynx
{

enum LogLevel
{
	TRACE = 0,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
	OFF
};

constexpr std::string_view logLevel2String(LogLevel level)
{
	switch (level)
	{
	case TRACE:
		return "TRACE";
	case DEBUG:
		return "DEBUG";
	case INFO:
		return "INFO";
	case WARN:
		return "WARN";
	case ERROR:
		return "ERROR";
	case FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

constexpr const char* logLevel2Color(LogLevel level)
{
	switch (level)
	{
	case TRACE:
		return "\033[90m";
	case DEBUG:
		return "\033[36m";
	case INFO:
		return "\033[32m";
	case WARN:
		return "\033[33m";
	case ERROR:
		return "\033[31m";
	case FATAL:
		return "\033[35m";
	default:
		return "\033[0m";
	}
}
} // namespace lynx

template <>
struct std::formatter<lynx::LogLevel> : std::formatter<std::string_view>
{
	auto format(lynx::LogLevel level, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(
			lynx::logLevel2String(level), ctx);
	}
};

namespace lynx
{

class NullLogger : public noncopyable
{
  public:
	NullLogger(LogLevel, const char*, const char*, int)
	{
	}

	template <typename T> NullLogger& operator<<(const T&)
	{
		return *this;
	}

	NullLogger& operator<<(std::ostream& (*)(std::ostream&))
	{
		return *this;
	}
};

class AsyncLogging;
class Logger : public noncopyable
{

  private:
	static std::unique_ptr<AsyncLogging> async_logging_;
	static std::atomic<bool> async_enabled_;

  public:
	/**
	 * 初始化异步日志系统
	 * @param log_file_base 日志文件路径以及前缀
	 * @param roll_size 日志文件滚动大小（字节），默认100MB
	 * @param flush_interval 缓冲区刷新间隔（秒），默认3秒
	 */
	static void initAsyncLogging(const std::string& log_file_base,
								 const std::string& exe_name,
								 int roll_size = 100 * 1024 * 1024,
								 int flush_interval = 3);

	static void shutdownAsyncLogging();

	static bool isAsyncEnabled();

  private:
	static void appendAsyncLog(LogLevel level, const std::string& message,
							   const char* file, const char* func, int line);

  public:
	class LogStream : noncopyable
	{
	  private:
		LogLevel level_;

		const char* file_;
		const char* func_;
		int line_;

	  public:
		LogStream(LogLevel level, const char* file, const char* func, int line)
			: level_(level), file_(file), func_(func), line_(line)
		{
		}

		~LogStream()
		{
			if (!CurrentThread::str().empty())
			{
				if (Logger::isAsyncEnabled())
				{
					Logger::appendAsyncLog(level_, CurrentThread::str(), file_,
										   func_, line_);
				}
				else
				{
					std::string formatted_log = std::format(
						"{}{}[{}]{} {}:{} {}()-> {}\033[0m\n",
						logLevel2Color(level_),
						TimeStamp::now().toFormattedString(),
						CurrentThread::tid(), level_, getShortName(file_),
						line_, func_, CurrentThread::str());

					std::cout << formatted_log;
				}
			}
			CurrentThread::str().clear();
		}

		template <typename T> LogStream& operator<<(const T& val)
		{
			std::format_to(std::back_inserter(CurrentThread::str()), "{}", val);
			return *this;
		}

		static const char* getShortName(const char* file)
		{
			assert(file);
			const char* short_name = std::strrchr(file, '/');
			if (short_name)
			{
				short_name++;
			}
			else
			{
				short_name = file;
			}

			return short_name;
		}
	};
};

#define LYNX_TRACE lynx::TRACE
#define LYNX_DEBUG lynx::DEBUG
#define LYNX_INFO lynx::INFO
#define LYNX_WARN lynx::WARN
#define LYNX_ERROR lynx::ERROR
#define LYNX_FATAL lynx::FATAL
#define LYNX_OFF lynx::OFF

#ifndef LOGGER_LEVEL_SETTING
#define LOGGER_LEVEL_SETTING LYNX_INFO
#endif

constexpr LogLevel GLOBAL_MIN_LEVEL =
	static_cast<LogLevel>(LOGGER_LEVEL_SETTING);

template <LogLevel level>
using SelectedLogStream = std::conditional_t<level >= GLOBAL_MIN_LEVEL,
											 Logger::LogStream, NullLogger>;

} // namespace lynx

#define LOG_TRACE                                                              \
	lynx::SelectedLogStream<lynx::TRACE>(lynx::TRACE, __FILE__, __func__,      \
										 __LINE__)
#define LOG_DEBUG                                                              \
	lynx::SelectedLogStream<lynx::DEBUG>(lynx::DEBUG, __FILE__, __func__,      \
										 __LINE__)
#define LOG_INFO                                                               \
	lynx::SelectedLogStream<lynx::INFO>(lynx::INFO, __FILE__, __func__,        \
										__LINE__)
#define LOG_WARN                                                               \
	lynx::SelectedLogStream<lynx::WARN>(lynx::WARN, __FILE__, __func__,        \
										__LINE__)
#define LOG_ERROR                                                              \
	lynx::SelectedLogStream<lynx::ERROR>(lynx::ERROR, __FILE__, __func__,      \
										 __LINE__)
#define LOG_FATAL                                                              \
	lynx::SelectedLogStream<lynx::FATAL>(lynx::FATAL, __FILE__, __func__,      \
										 __LINE__)

#endif
