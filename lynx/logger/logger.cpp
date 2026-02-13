#include "lynx/logger/logger.h"
#include "lynx/base/current_thread.hpp"
#include "lynx/logger/async_logging.h"
#include "lynx/logger/context.hpp"
#include "lynx/time/time_stamp.h"
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <pthread.h>

using namespace lynx;

std::unique_ptr<AsyncLogging> Logger::async_logging_ = nullptr;
std::atomic<bool> Logger::async_enabled_(false);

void Logger::initAsyncLogging(const std::string& log_file_base,
							  const std::string& exe_name, int roll_size,
							  int flush_interval)

{
	assert(roll_size > 0 && flush_interval > 0);

	if (async_enabled_.load(std::memory_order_acquire))
	{
		return;
	}

	static std::once_flag flag;

	std::call_once(flag,
				   [&]()
				   {
					   async_logging_ = std::make_unique<AsyncLogging>(
						   log_file_base,
						   LogStream::getShortName(exe_name.c_str()), roll_size,
						   flush_interval);
					   async_enabled_.store(true, std::memory_order_release);
				   });
}

void Logger::shutdownAsyncLogging()
{
	if (!async_enabled_.load(std::memory_order_acquire))
	{
		return;
	}

	static std::once_flag flag;

	std::call_once(flag,
				   [&]()
				   {
					   async_logging_.reset();
					   async_enabled_.store(false, std::memory_order_release);
				   });
}

bool Logger::isAsyncEnabled()
{
	return async_enabled_.load(std::memory_order_acquire);
}

void Logger::appendAsyncLog(LogLevel level, const std::string& message,
							const char* file, const char* func, int line)
{
	if (!isAsyncEnabled())
	{
		return;
	}

	thread_local Context ctx(CurrentThread::tid());

	ctx.withTimeStamp(TimeStamp::now())
		.withLevel(level)
		.WithData(Context::Data{.line = line,
								.full_name = file,
								.short_name = LogStream::getShortName(file),
								.func = func})
		.withText(message);

	async_logging_->pushMessage(ctx);
}