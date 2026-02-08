#include "lynx/logger/log_file.h"
#include "lynx/logger/file_appender.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <mutex>

namespace lynx
{
constexpr int kSumLength = 1024;

thread_local char t_filename[kSumLength];
thread_local char t_time[64];
thread_local int t_timezone = -1;
thread_local std::tm t_tm;
thread_local std::tm t_gmtm;
thread_local time_t t_lastSeconds;

const char* getHostName()
{
	static char buf[256];
	static bool initialized = false;
	if (!initialized)
	{
		if (::gethostname(buf, sizeof(buf)) == 0)
		{
			buf[sizeof(buf) - 1] = '\0';
		}
		else
		{
			::strncpy(buf, "unknown_host", sizeof(buf));
		}
		initialized = true;
	}
	return buf;
}

int getPid()
{
	static int pid = ::getpid();
	return pid;
}

const char* getFilename(const char* basename, const time_t& now)
{
	auto basename_len = ::strlen(basename);
	assert(basename_len < kSumLength - 200);
	t_filename[basename_len] = '\0';
	char* filename = t_filename;

	::strncpy(filename, basename, basename_len);
	assert(filename && filename[basename_len] == '\0');

	size_t remaining_len = kSumLength - basename_len;
	std::tm tm{};

	::localtime_r(&now, &tm);
	size_t tsz = std::strftime(filename + basename_len, remaining_len,
							   ".%Y%m%d-%H%M%S.", &tm);

	size_t nsz =
		std::snprintf(filename + basename_len + tsz, remaining_len - tsz,
					  "%s.%d.log", getHostName(), getPid());

	assert(filename[basename_len + tsz + nsz] == '\0');

	return filename;
}

LogFile::LogFile(const std::string& basename, int roll_size, int flush_interval,
				 int check_every)
	: basename_(basename), roll_size_(roll_size),
	  flush_interval_(flush_interval), check_every_(check_every)
{
	rollFile();
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* line, size_t len)
{
	std::lock_guard<std::mutex> lock(mtx_);
	appendWOLock(line, len);
}

void LogFile::flush()
{
	std::lock_guard<std::mutex> lock(mtx_);
	file_->flush();
}

void LogFile::rollFile(const time_t* cached_now)
{
	time_t now;
	if (cached_now != nullptr)
	{
		now = *cached_now;
	}
	else
	{
		now = ::time(nullptr);
	}

	const char* filename = getFilename(basename_.c_str(), now);
	auto start = now / kRollPerSeconds * kRollPerSeconds; // 取整

	if (now > last_roll_)
	{
		last_roll_ = now;
		last_flush_ = now;
		last_period_ = start;

		file_ = std::make_unique<FileAppender>(filename);
	}
}

void LogFile::appendWOLock(const char* line, size_t len)
{
	file_->append(line, len);
	if (file_->writtenBytes() > roll_size_)
	{
		rollFile();
		file_->resetWritten();
	}
	else
	{
		count_++;
		if (count_ >= check_every_)
		{
			count_ = 0;
			time_t now = ::time(nullptr);
			time_t cur_period = now / kRollPerSeconds * kRollPerSeconds;
			if (cur_period != last_period_)
			{
				rollFile(&now);
			}
			else if (now - last_flush_ >= flush_interval_)
			{
				last_flush_ = now;
				file_->flush();
			}
		}
	}
}
} // namespace lynx