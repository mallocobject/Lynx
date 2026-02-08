#include "lynx/base/time_stamp.h"
#include <chrono>
#include <cstdint>
#include <ctime>
#include <format>

namespace lynx
{
extern const int kMicroSecond2Second = 1e6 * 1.0;

TimeStamp TimeStamp::now()
{
	auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now());

	int64_t micro_seconds =
		std::chrono::duration_cast<std::chrono::microseconds>(
			now_us.time_since_epoch())
			.count();

	// auto seconds =
	// std::chrono::time_point_cast<std::chrono::seconds>(now_us);

	// auto micro_seconds = now_us - seconds;

	// int curent_second = std::chrono::system_clock::to_time_t(seconds);

	// if (cached_time_.last_second != curent_second)
	// {
	// 	// todo
	// }

	// static thread_local std::string final_time_stamp;
	// final_time_stamp = std::format(
	// 	"{}{:06d}", cached_time_.formatted_time_stamp,
	// micro_seconds.count());

	return TimeStamp(micro_seconds);
}
TimeStamp TimeStamp::addTime(TimeStamp time_stamp, double add_seconds)
{
	int64_t delta = static_cast<int64_t>(add_seconds * kMicroSecond2Second);

	return TimeStamp(time_stamp.micro_seconds_ + delta);
}

std::string TimeStamp::toFormattedString(bool date, bool time) const
{
	if (!date && !time)
	{
		return "";
	}

	int64_t seconds = micro_seconds_ / kMicroSecond2Second;
	int64_t remaining_micro_seconds = micro_seconds_ % kMicroSecond2Second;

	tm tm_time;
	localtime_r(&seconds, &tm_time);

	if (date && time)
	{
		return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:06d}",
						   tm_time.tm_year + 1900, tm_time.tm_mon + 1,
						   tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min,
						   tm_time.tm_sec, remaining_micro_seconds);
	}
	else if (date)
	{
		return std::format("{:04d}-{:02d}-{:02d}", tm_time.tm_year + 1900,
						   tm_time.tm_mon + 1, tm_time.tm_mday);
	}

	return std::format("{:02d}:{:02d}:{:02d}.{:06d}", tm_time.tm_hour,
					   tm_time.tm_min, tm_time.tm_sec, remaining_micro_seconds);
}

std::ostream& operator<<(std::ostream& os, const TimeStamp& ts)
{
	os << ts.toFormattedString();
	return os;
}
} // namespace lynx