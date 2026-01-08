#include "lynx/include/logger.h"
#include "lynx/include/time_stamp.h"
#include <iostream>

namespace lynx
{
Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

void Logger::log(const std::string& msg)
{
	switch (log_level_)
	{
	case DEBUG:
		std::cout << "\033[36m[DEBUG]";
		break;
	case INFO:
		std::cout << "\033[32m[INFO]";
		break;
	case WARN:
		std::cout << "\033[33m[WARN]";
		break;
	case ERROR:
		std::cout << "\033[31m[ERROR]";
		break;
	case FATAL:
		std::cout << "\033[35m[FATAL]";
		break;
	default:
		std::cout << "\033[0m[UNKNOWN]";
		break;
	}

	std::cout << TimeStamp::now() << " : " << msg << std::endl;
}

} // namespace lynx