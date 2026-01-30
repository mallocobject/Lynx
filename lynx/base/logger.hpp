#ifndef LYNX_LOGGER_HPP
#define LYNX_LOGGER_HPP

#include "lynx/base/common.hpp"
#include "lynx/base/time_stamp.h"
#include <iostream>
#include <sstream>

namespace lynx
{

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
	enum Level
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};

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
				switch (level_)
				{
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

		~LogStream()
		{
			if (enabled_ && !stream_.str().empty())
			{
				stream_ << "\033[0m" << std::endl;
				std::cout << stream_.str();
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
	static LogStream log(Level level)
	{
		return LogStream(level);
	}

#ifndef NODEBUG
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

	static LogStream info()
	{
		return LogStream(INFO);
	}

	static LogStream warn()
	{
		return LogStream(WARN);
	}

	static LogStream error()
	{
		return LogStream(ERROR);
	}

	static LogStream fatal()
	{
		return LogStream(FATAL);
	}
};

#define LOG_DEBUG Logger::debug()
#define LOG_INFO Logger::info()
#define LOG_WARN Logger::warn()
#define LOG_ERROR Logger::error()
#define LOG_FATAL Logger::fatal()

} // namespace lynx

#endif // LYNX_LOGGER_H