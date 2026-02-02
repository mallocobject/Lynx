#ifndef LYNX_LOGGER_HPP
#define LYNX_LOGGER_HPP

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace lynx
{

// 前向声明
class AsyncLogging;
class Formatter;
class Context;

// 定义等级数值
#define LYNX_LOG_LEVEL_TRACE 0
#define LYNX_LOG_LEVEL_DEBUG 1
#define LYNX_LOG_LEVEL_INFO 2
#define LYNX_LOG_LEVEL_WARN 3
#define LYNX_LOG_LEVEL_ERROR 4
#define LYNX_LOG_LEVEL_FATAL 5
#define LYNX_LOG_LEVEL_OFF 6

// --- 用户配置区 ---
// 如果外部没有定义 LYNX_MIN_LOG_LEVEL，则根据编译模式设置默认值
#ifndef LYNX_MIN_LOG_LEVEL
#ifdef LYNX_TRACE_ON
#define LYNX_MIN_LOG_LEVEL LYNX_LOG_LEVEL_TRACE
#elif !defined(NODEBUG)
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

/**
 * 重构后的日志系统
 * 支持：
 * 1. 控制台彩色输出（原有功能）
 * 2. 异步文件写入（新增功能）
 * 3. 自定义日志格式（新增功能）
 */
class Logger
{
  public:
	enum Level
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

  private:
	class LogStream
	{
	  private:
		Level level_;
		std::ostringstream stream_;
		bool enabled_;

	  public:
		DISABLE_COPY(LogStream)

		LogStream(Level level, bool enabled = true)
			: level_(level), enabled_(enabled)
		{
			if (enabled_)
			{
				// 仅在输出到控制台时添加前缀
				// 如果输出到文件，Formatter 会统一处理格式
				if (!Logger::isAsyncEnabled())
				{
					switch (level_)
					{
					case TRACE:
						stream_ << "\033[90m[TRACE]";
						break;
					case DEBUG:
						stream_ << "\033[36m[DEBUG]";
						break;
					case INFO:
						stream_ << "\033[32m[INFO]";
						break;
					case WARN:
						stream_ << "\033[33m[WARN]";
						break;
					case ERROR:
						stream_ << "\033[31m[ERROR]";
						break;
					case FATAL:
						stream_ << "\033[35m[FATAL]";
						break;
					default:
						stream_ << "\033[0m[UNKNOWN]";
						break;
					}
					stream_ << TimeStamp::now() << " : ";
				}
			}
		}

		~LogStream()
		{
			if (enabled_ && !stream_.str().empty())
			{
				std::string log_msg = stream_.str();

				if (Logger::isAsyncEnabled())
				{
					// 输出到文件 - 仅传递原始消息，Formatter 处理格式
					Logger::pushAsyncLog(level_, log_msg);
				}
				else
				{
					// 输出到控制台 - 添加结束标记和换行
					log_msg += "\033[0m\n";
					std::cout << log_msg;
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
#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_TRACE
	static LogStream trace()
	{
		return LogStream(TRACE);
	}
#else
	static NullLogger trace()
	{
		return null_logger;
	}
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_DEBUG
	static LogStream debug()
	{
		return LogStream(DEBUG);
	}
#else
	static NullLogger debug()
	{
		return null_logger;
	}
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_INFO
	static LogStream info()
	{
		return LogStream(INFO);
	}
#else
	static NullLogger info()
	{
		return null_logger;
	}
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_WARN
	static LogStream warn()
	{
		return LogStream(WARN);
	}
#else
	static NullLogger warn()
	{
		return null_logger;
	}
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_ERROR
	static LogStream error()
	{
		return LogStream(ERROR);
	}
#else
	static NullLogger error()
	{
		return null_logger;
	}
#endif

#if LYNX_MIN_LOG_LEVEL <= LYNX_LOG_LEVEL_FATAL
	static LogStream fatal()
	{
		return LogStream(FATAL);
	}
#else
	static NullLogger fatal()
	{
		return null_logger;
	}
#endif

  private:
	static std::unique_ptr<AsyncLogging> async_logging_;
	static std::unique_ptr<Formatter> formatter_;
	static std::mutex mutex_;
	static bool async_enabled_;

	// 内部方法：发送日志到异步系统
	static void pushAsyncLog(Level level, const std::string& message);
};

#define LOG_TRACE Logger::trace()
#define LOG_DEBUG Logger::debug()
#define LOG_INFO Logger::info()
#define LOG_WARN Logger::warn()
#define LOG_ERROR Logger::error()
#define LOG_FATAL Logger::fatal()

} // namespace lynx

#endif // LYNX_LOGGER_H