#ifndef LYNX_LOGGER_LOGGER_HPP
#define LYNX_LOGGER_LOGGER_HPP

#include "lynx/base/current_thread.hpp"
#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.hpp"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>
#include <type_traits>
namespace lynx
{
namespace logger
{
enum class LogLevel : uint8_t
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
	case LogLevel::TRACE:
		return "TRACE";
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARN:
		return "WARN";
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

constexpr const char* logLevel2Color(LogLevel level)
{
	switch (level)
	{
	case LogLevel::TRACE:
		return "\033[90m";
	case LogLevel::DEBUG:
		return "\033[36m";
	case LogLevel::INFO:
		return "\033[32m";
	case LogLevel::WARN:
		return "\033[33m";
	case LogLevel::ERROR:
		return "\033[31m";
	case LogLevel::FATAL:
		return "\033[35m";
	default:
		return "\033[0m";
	}
}
} // namespace logger
} // namespace lynx

template <>
struct std::formatter<lynx::logger::LogLevel> : std::formatter<std::string_view>
{
	auto format(lynx::logger::LogLevel level, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(
			lynx::logger::logLevel2String(level), ctx);
	}
};

namespace lynx
{
namespace logger
{
class NullLogger : public base::noncopyable
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
class Logger : public base::noncopyable
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
			if (!base::CurrentThread::str().empty())
			{
				if (Logger::isAsyncEnabled())
				{
					Logger::appendAsyncLog(level_, base::CurrentThread::str(),
										   file_, func_, line_);
				}
				else
				{
					std::string formatted_log = std::format(
						"{}{}[{}]{} {}:{} {}()-> {}\033[0m\n",
						logLevel2Color(level_),
						time::TimeStamp::now().toFormattedString(),
						base::CurrentThread::tid(), level_, getShortName(file_),
						line_, func_, base::CurrentThread::str());

					std::cout << formatted_log;
				}
			}
			base::CurrentThread::str().clear();
		}

		template <typename T> LogStream& operator<<(const T& val)
		{
			std::format_to(std::back_inserter(base::CurrentThread::str()), "{}",
						   val);
			return *this;
		}

		// LogStream& operator<<(std::ostream& (*manip)(std::ostream))
		// {
		// 	CurrentThread::oss() << manip;
		// 	return *this;
		// }

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

#define LYNX_TRACE LogLevel::TRACE
#define LYNX_DEBUG LogLevel::DEBUG
#define LYNX_INFO LogLevel::INFO
#define LYNX_WARN LogLevel::WARN
#define LYNX_ERROR LogLevel::ERROR
#define LYNX_FATAL LogLevel::FATAL
#define LYNX_OFF LogLevel::OFF

#ifndef LOGGER_LEVEL_SETTING
#define LOGGER_LEVEL_SETTING LYNX_INFO
#endif

constexpr LogLevel GLOBAL_MIN_LEVEL =
	static_cast<LogLevel>(LOGGER_LEVEL_SETTING);

template <LogLevel level>
using SelectedLogStream = std::conditional_t<level >= GLOBAL_MIN_LEVEL,
											 Logger::LogStream, NullLogger>;
} // namespace logger
} // namespace lynx

#define LOG_TRACE                                                              \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::TRACE>(            \
		lynx::logger::LogLevel::TRACE, __FILE__, __func__, __LINE__)
#define LOG_DEBUG                                                              \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::DEBUG>(            \
		lynx::logger::LogLevel::DEBUG, __FILE__, __func__, __LINE__)
#define LOG_INFO                                                               \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::INFO>(             \
		lynx::logger::LogLevel::INFO, __FILE__, __func__, __LINE__)
#define LOG_WARN                                                               \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::WARN>(             \
		lynx::logger::LogLevel::WARN, __FILE__, __func__, __LINE__)
#define LOG_ERROR                                                              \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::ERROR>(            \
		lynx::logger::LogLevel::ERROR, __FILE__, __func__, __LINE__)
#define LOG_FATAL                                                              \
	lynx::logger::SelectedLogStream<lynx::logger::LogLevel::FATAL>(            \
		lynx::logger::LogLevel::FATAL, __FILE__, __func__, __LINE__)

#endif
