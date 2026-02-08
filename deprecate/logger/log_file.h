#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "lynx/base/common.hpp"
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
namespace lynx
{
class FileAppender;
class LogFile
{
  private:
	static constexpr int kRollPerSeconds = 24 * 60 * 60;
	std::string basename_;
	std::unique_ptr<FileAppender> file_;
	std::mutex mtx_;
	time_t last_period_; //  单位为天
	time_t last_roll_;	 // 上一次 roll 日志的时间（精确到秒）
	time_t last_flush_;	 // 上一次 flush 的时间

	const int roll_size_;
	const int flush_interval_;
	const int check_every_;
	int count_;

  public:
	DISABLE_COPY(LogFile)

	explicit LogFile(const std::string& basename, int roll_size,
					 int flush_interval = 3, int check_every = 1024);
	~LogFile();

	void append(const char* line, size_t len);
	void flush();

  private:
	void appendWOLock(const char* line, size_t len);
	void rollFile(const time_t* cached_now = nullptr);
};
} // namespace lynx

#endif