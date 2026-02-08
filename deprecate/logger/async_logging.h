#ifndef LYNX_ASYNC_LOGGING_H
#define LYNX_ASYNC_LOGGING_H

#include "lynx/base/common.hpp"
#include "lynx/logger/context.hpp"
#include "lynx/logger/fixed_buffer.hpp"
#include "lynx/logger/formatter.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
namespace lynx
{
enum
{
	kSmallBuffer = 4096,
	kLargeBuffer = 65536
};

class AsyncLogging
{
	using Buffer = FixedBuffer<kLargeBuffer>;

  private:
	const int flush_interval_;
	const int roll_size_;
	std::atomic<bool> done_;
	std::once_flag once_flag_;
	std::string basename_;
	std::unique_ptr<std::thread> thread_;
	std::mutex mtx_;
	std::condition_variable cv_;

	std::mutex mtx_for_init_;
	std::condition_variable cv_for_init_;
	std::atomic<bool> flag_for_init_;

	Formatter formatter_;

	Buffer cur_buffer_;
	Buffer next_buffer_;
	std::vector<Buffer> buffers_;

  public:
	DISABLE_COPY(AsyncLogging)

	explicit AsyncLogging(const std::string& basename, int roll_size,
						  int flush_interval = 3);

	~AsyncLogging();

	void pushMessage(const Context& ctx);

	void waitForDone();

  private:
	void doDone();

	void threadWorker();
};
} // namespace lynx

#endif