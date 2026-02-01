#define LYNX_MIN_LOG_LEVEL 4

#include "lynx/logger/logger.hpp"

int main()
{
	lynx::LOG_TRACE << "Hello world";
	lynx::LOG_DEBUG << "Hello world";
	lynx::LOG_INFO << "Hello world";
	lynx::LOG_WARN << "Hello world";
	lynx::LOG_ERROR << "Hello world";
	lynx::LOG_FATAL << "Hello world";
}