#include "lynx/logger/async_logging.h"
#include "lynx/base/time_stamp.h"
#include "lynx/logger/log_file.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace lynx
{
AsyncLogging::AsyncLogging(const std::string& basename, int roll_size,
						   int flush_interval)
	: flush_interval_(flush_interval), roll_size_(roll_size),
	  basename_(basename), done_(false), flag_for_init_(false)
{
	thread_ = std::make_unique<std::thread>(
		std::bind(&AsyncLogging::threadWorker, this));
	{
		std::unique_lock<std::mutex> lock(mtx_for_init_);
		cv_for_init_.wait(
			lock,
			[this] { return flag_for_init_.load(std::memory_order_acquire); });
	}
}

AsyncLogging::~AsyncLogging()
{
	if (done_.load(std::memory_order_acquire))
	{
		return;
	}
	doDone();
}

void AsyncLogging::pushMessage(const Context& ctx)
{
	assert(!done_);

	std::lock_guard<std::mutex> lock(mtx_);
	if (!cur_buffer_.full())
	{
		cur_buffer_.push(ctx);
		return;
	}

	buffers_.push_back(std::move(cur_buffer_));
	if (next_buffer_.check())
	{
		cur_buffer_ = std::move(next_buffer_);
	}
	else
	{
		cur_buffer_ = Buffer();
	}

	cur_buffer_.push(ctx);
	cv_.notify_one();
}

void AsyncLogging::waitForDone()
{
	if (done_.load(std::memory_order_acquire))
	{
		return;
	}
	doDone();
}

void AsyncLogging::doDone()
{
	done_.store(true, std::memory_order_release);
	cv_.notify_one();

	if (thread_ && thread_->joinable())
	{
		thread_->join();
	}
}

void AsyncLogging::threadWorker()
{
	LogFile out_file(basename_, roll_size_, false);
	Buffer new_buffer1;
	Buffer new_buffer2;

	std::vector<Buffer> buffer2write;
	buffer2write.reserve(16);

	while (!done_.load(std::memory_order_acquire))
	{
		{
			std::unique_lock<std::mutex> lock(mtx_);
			if (buffers_.empty())
			{
				std::call_once(once_flag_,
							   [&]()
							   {
								   flag_for_init_.store(
									   true, std::memory_order_release);
								   cv_for_init_.notify_one();
							   });

				cv_.wait_for(lock,
							 std::chrono::seconds(
								 flush_interval_)); // roll file every
													// flush_interval_ seconds
			}

			if (!cur_buffer_.empty())
			{
				buffers_.push_back(std::move(cur_buffer_));
				cur_buffer_ = std::move(new_buffer1);
			}

			buffer2write.swap(buffers_);

			if (!new_buffer2.check())
			{
				next_buffer_ = std::move(new_buffer2);
			}
		}

		if (buffer2write.empty())
		{
			continue;
		}

		if (buffer2write.size() > 100)
		{
			char buf[256];
			snprintf(buf, sizeof buf,
					 "Dropped log messages at %s, %zd larger buffers\n",
					 TimeStamp::now().toString().c_str(),
					 buffer2write.size() - 2);
			fputs(buf, stderr);

			out_file.append(buf, static_cast<int>(strlen(buf)));
			buffer2write.erase(buffer2write.begin() + 2, buffer2write.end());
		}

		for (auto& buffer : buffer2write)
		{
			for (size_t i = 0; i < buffer.size(); i++)
			{
				// out_file.append(buffer.);
			}
		}

		if (buffer2write.size() < 2)
		{
			buffer2write.resize(2);
		}

		if (!new_buffer1.check())
		{
			assert(!buffer2write.empty());
			new_buffer1 = std::move(buffer2write[0]);
			new_buffer1.reset();
		}

		if (!new_buffer2.check())
		{
			assert(!buffer2write.empty());
			new_buffer2 = std::move(buffer2write[1]);
			new_buffer2.reset();
		}

		buffer2write.clear();
		out_file.flush();
	}

	out_file.flush();
}
} // namespace lynx