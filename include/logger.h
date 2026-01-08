#ifndef LYNX_LOGGER_H
#define LYNX_LOGGER_H

#include "lynx/include/common.h"
#include <sstream>
#include <string>

namespace lynx
{

class NullLogger
{
  public:
	template <typename T> NullLogger& operator<<(const T&)
	{
		return *this;
	}
};

inline NullLogger null_logger;

#define LOG(level) Logger::instance().setLogLevel(level)

#define LOG_INFO() LOG(lynx::Logger::INFO)
#define LOG_WARN() LOG(lynx::Logger::WARN)
#define LOG_ERROR() LOG(lynx::Logger::ERROR)
#define LOG_FATAL() LOG(lynx::Logger::FATAL)

#ifndef NODEBUG
#define LOG_DEBUG() LOG(lynx::Logger::DEBUG)
#else
#define LOG_DEBUG() null_logger
#endif

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
	Level log_level_;
	std::ostringstream stream_;
	Logger() = default;

  public:
	DISABLE_COPY_AND_MOVE(Logger)

	static Logger& instance();
	~Logger() = default;

	Logger& setLogLevel(Level level)
	{
		log_level_ = level;
		return *this;
	}

	void log(const std::string& msg);

	template <typename T> Logger& operator<<(const T& val);
};

template <typename T> Logger& Logger::operator<<(const T& val)
{
	stream_ << val;
	log(stream_.str());
	stream_.str("");
	stream_.clear();
	return *this;
}

} // namespace lynx

#endif