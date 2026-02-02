#ifndef LYNX_LOGGER_HPP
#define LYNX_LOGGER_HPP

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <syncstream>

namespace lynx
{

// 前向声明
class AsyncLogging;

// 定义等级数值
#define LYNX_LOG_LEVEL_DEBUG 0
#define LYNX_LOG_LEVEL_INFO 1
#define LYNX_LOG_LEVEL_WARN 2
#define LYNX_LOG_LEVEL_ERROR 3
#define LYNX_LOG_LEVEL_FATAL 4
#define LYNX_LOG_LEVEL_OFF 5

// --- 用户配置区 ---
#ifndef LYNX_MIN_LOG_LEVEL
#if !defined(NODEBUG)
#define LYNX_MIN_LOG_LEVEL LYNX_LOG_LEVEL_DEBUG
#else
#define LYNX_MIN_LOG_LEVEL LYNX_LOG_LEVEL_INFO
#endif
#endif

class NullLogger
{
  public:
	template <typename T> NullLogger& operator<<(const T&)
	{
		return *this;
	}

	// for std::endl and std::flush
	NullLogger& operator<<(std::ostream& (*manip)(std::ostream&))
	{
		return *this;
	}
};

inline NullLogger null_logger;

class Logger
{
  public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};

	/**
	 * 初始化异步日志系统
	 * @param log_file 日志文件路径
	 * @param roll_size 日志文件滚动大小（字节），默认100MB
	 * @param flush_interval 缓冲区刷新间隔（秒），默认3秒
	 */
	static void initAsyncLogging(const std::string& log_file,
								 int roll_size = 100 * 1024 * 1024,
								 int flush_interval = 3);

	/**
	 * 关闭异步日志并等待所有日志写入完成
	 */
	static void shutdownAsyncLogging();

	/**
	 * 检查异步日志是否已启用
	 */
	static bool isAsyncEnabled();

  public:
	class LogStream
	{
	  private:
		LogLevel level_;
		std::ostringstream stream_;
		bool enabled_;

		const char* file_;
		const char* func_;
		int line_;

	  public:
		DISABLE_COPY(LogStream)

		LogStream(LogLevel level, const char* file, const char* func, int line,
				  bool enabled = true)
			: level_(level), enabled_(enabled), file_(file), func_(func),
			  line_(line)
		{
		}

		LogStream& setSourceInfo(const char* file, const char* func, int line)
		{
			file_ = file;
			func_ = func;
			line_ = line;
			return *this;
		}

		~LogStream()
		{
			if (enabled_ && !stream_.str().empty())
			{
				if (Logger::isAsyncEnabled())
				{
					// 输出到文件 - 传递消息和源位置信息给 Formatter
					Logger::pushAsyncLog(level_, stream_.str(), file_, func_,
										 line_);
				}
				else
				{
					// 输出到控制台 - 添加结束标记和换行
					std::string prefix;
					switch (level_)
					{
					case TRACE:
						prefix = "\033[90m[TRACE]";
						break;
					case DEBUG:
						prefix = "\033[36m[DEBUG]";
						break;
					case INFO:
						prefix = "\033[32m[INFO]";
						break;
					case WARN:
						prefix = "\033[33m[WARN]";
						break;
					case ERROR:
						prefix = "\033[31m[ERROR]";
						break;
					case FATAL:
						prefix = "\033[35m[FATAL]";
						break;
					default:
						prefix = "\033[0m[UNKNOWN]";
						break;
					}
					prefix += TimeStamp::now().toFormattedString();
					prefix += " : ";
					std::osyncstream(std::cout)
						<< prefix << stream_.str() << "\033[0m\n";
				}
			}
		}

		// allow move
		LogStream(LogStream&& other) noexcept
			: level_(other.level_), enabled_(other.enabled_)
		{
			stream_ = std::move(other.stream_);
		}

		template <typename T> LogStream& operator<<(const T& val)
		{
			if (enabled_)
			{
				stream_ << val;
			}
			return *this;
		}

		// for std::endl and std::flush
		LogStream& operator<<(std::ostream& (*manip)(std::ostream&))
		{
			if (enabled_)
			{
				stream_ << manip;
			}
			return *this;
		}
	};

  public:
#if LYNX_LOG_TRACE_ON
#define LOG_TRACE                                                              \
	Logger::LogStream(lynx::Logger::TRACE, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_TRACE null_logger
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_DEBUG
#define LOG_DEBUG                                                              \
	Logger::LogStream(lynx::Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_DEBUG null_logger
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_INFO
#define LOG_INFO                                                               \
	Logger::LogStream(lynx::Logger::INFO, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_INFO null_logger
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_WARN
#define LOG_WARN                                                               \
	Logger::LogStream(lynx::Logger::WARN, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_WARN null_logger
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_ERROR
#define LOG_ERROR                                                              \
	Logger::LogStream(lynx::Logger::ERROR, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_ERROR null_logger
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_FATAL
#define LOG_FATAL                                                              \
	Logger::LogStream(lynx::Logger::FATAL, __FILE__, __FUNCTION__, __LINE__)
#else
#define LOG_FATAL null_logger
#endif

  private:
	static std::unique_ptr<AsyncLogging> async_logging_;
	static std::atomic<bool> async_enabled_;
	static std::mutex mtx_;

	// 内部方法：发送日志到异步系统
	static void pushAsyncLog(LogLevel level, const std::string& message,
							 const char* file, const char* func, int line);
};

} // namespace lynx

#endif // LYNX_LOGGER_H