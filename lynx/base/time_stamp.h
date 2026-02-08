#ifndef LYNX_TIME_STAMP_H
#define LYNX_TIME_STAMP_H

#include <cstdint>
#include <ostream>
#include <string>
namespace lynx
{
extern const int kMicroSecond2Second;

class TimeStamp
{
  private:
	// struct CachedTime
	// {
	// 	std::string formatted_time_stamp;
	// 	std::chrono::microseconds last_update;
	// 	int last_second;
	// };

	// static thread_local CachedTime cached_time_;

	int64_t micro_seconds_{0};

  public:
	TimeStamp() = default;

	explicit TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds)
	{
	}

	~TimeStamp() = default;

	static TimeStamp now();
	static TimeStamp addTime(TimeStamp time_stamp, double add_seconds);

	std::string toFormattedString(bool date = true, bool time = true) const;

	int64_t microseconds() const
	{
		return micro_seconds_;
	}

	auto operator<=>(const TimeStamp& rhs) const = default;

	friend std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);
};

std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);

} // namespace lynx

#endif