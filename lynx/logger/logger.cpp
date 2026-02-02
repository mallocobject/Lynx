#include "lynx/logger/logger.h"
#include "lynx/base/current_thread.hpp"
#include "lynx/logger/async_logging.h"
#include "lynx/logger/context.hpp"
#include <atomic>
#include <cstring>
#include <memory>

namespace lynx
{

// 静态成员初始化
std::unique_ptr<AsyncLogging> Logger::async_logging_ = nullptr;
std::atomic<bool> Logger::async_enabled_ = false;
std::mutex Logger::mtx_;

void Logger::initAsyncLogging(const std::string& log_file, int roll_size,
							  int flush_interval)
{
	std::lock_guard<std::mutex> lock(mtx_);

	if (async_enabled_.load(std::memory_order_acquire))
	{
		return;
	}

	try
	{
		async_logging_ =
			std::make_unique<AsyncLogging>(log_file, roll_size, flush_interval);
		async_enabled_.store(true, std::memory_order_release);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to initialize AsyncLogging: " << e.what()
				  << std::endl;
		async_logging_ = nullptr;
		async_enabled_.store(false, std::memory_order_release);
	}
}

void Logger::shutdownAsyncLogging()
{
	std::lock_guard<std::mutex> lock(mtx_);

	if (async_enabled_.load(std::memory_order_acquire))
	{
		async_logging_->waitForDone();
		async_logging_.reset();
		async_enabled_.store(false, std::memory_order_release);
	}
}

bool Logger::isAsyncEnabled()
{
	return async_enabled_.load(std::memory_order_relaxed);
}

void Logger::pushAsyncLog(LogLevel level, const std::string& message,
						  const char* file, const char* func, int line)
{

	if (!async_enabled_.load(std::memory_order_acquire))
	{
		return;
	}

	Context ctx;
	ctx.level = static_cast<int>(level);
	ctx.tid = CurrentThread::tid();
	ctx.timestamp = TimeStamp::now().microseconds();
	ctx.text = message;

	// 设置源文件信息
	if (file)
	{
		// 提取短文件名（去掉路径）
		const char* short_name = std::strrchr(file, '/');
		if (short_name)
		{
			short_name++;
		}
		else
		{
			short_name = file;
		}

		ctx.data.short_filename = short_name;
		ctx.data.full_filename = file;
	}

	ctx.data.func_name = func ? func : "";
	ctx.data.line = line;

	{
		std::lock_guard<std::mutex> lock(mtx_);
		async_logging_->pushMessage(ctx);
	}
}

} // namespace lynx
