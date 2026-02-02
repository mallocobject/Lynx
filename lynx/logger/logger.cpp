#include "lynx/logger/logger.h"
#include "lynx/logger/async_logging.h"
#include "lynx/logger/context.hpp"
#include "lynx/logger/formatter.hpp"
#include <thread>

namespace lynx
{

// 静态成员初始化
std::unique_ptr<AsyncLogging> Logger::async_logging_ = nullptr;
std::unique_ptr<Formatter> Logger::formatter_ = nullptr;
std::mutex Logger::mutex_;
bool Logger::async_enabled_ = false;

/**
 * 线程局部存储 - 缓存线程ID，避免每次都调用 std::this_thread::get_id()
 * get_id() 在某些平台上比较耗时，通过缓存减少系统调用
 */
thread_local std::thread::id thread_local_id = std::this_thread::get_id();

void Logger::initAsyncLogging(const std::string& log_file, int roll_size,
							  int flush_interval)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (async_logging_)
	{
		return; // 已经初始化过
	}

	try
	{
		async_logging_ =
			std::make_unique<AsyncLogging>(log_file, roll_size, flush_interval);
		formatter_ = std::make_unique<Formatter>(Flags::kStdFlags);
		async_enabled_ = true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to initialize AsyncLogging: " << e.what()
				  << std::endl;
		async_logging_ = nullptr;
		formatter_ = nullptr;
		async_enabled_ = false;
	}
}

void Logger::shutdownAsyncLogging()
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (async_logging_)
	{
		async_logging_->waitForDone();
		async_logging_.reset();
		formatter_.reset();
		async_enabled_ = false;
	}
}

bool Logger::isAsyncEnabled()
{
	return async_enabled_;
}

void Logger::pushAsyncLog(Level level, const std::string& message,
						  const char* file, const char* func, int line)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!async_logging_)
	{
		return;
	}

	Context ctx;
	ctx.level = static_cast<int>(level);
	ctx.tid = std::hash<std::thread::id>{}(thread_local_id);
	ctx.timestamp = TimeStamp::now().microseconds();
	ctx.text = message;

	// 设置源文件信息
	if (file)
	{
		// 提取短文件名（去掉路径）
		const char* short_name = file;
		for (const char* p = file; *p; ++p)
		{
			if (*p == '/')
			{
				short_name = p + 1;
			}
		}

		ctx.data.short_filename = short_name;
		ctx.data.full_filename = file;
	}

	ctx.data.func_name = func ? func : "";
	ctx.data.line = line;

	async_logging_->pushMessage(ctx);
}

} // namespace lynx
