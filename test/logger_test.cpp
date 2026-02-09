#define LOGGER_LEVEL_SETTING LYNX_TRACE

#include "lynx/logger/logger.h"
int main()
{
	LOG_TRACE << "hello world!";
	LOG_DEBUG << "hello world!";
	LOG_INFO << "hello world!";
	LOG_WARN << "hello world!";
	LOG_ERROR << "hello world!";
	LOG_FATAL << "hello world!";
}