// #define NODEBUG

#include "lynx/include/logger.h"
#include <unistd.h>

int main()
{
	lynx::LOG_INFO() << "hello world";
	lynx::LOG_WARN() << "hello world";
	lynx::LOG_ERROR() << "hello world";
	lynx::LOG_FATAL() << "hello world";
	lynx::LOG_DEBUG() << "hello world";

	// todo
}