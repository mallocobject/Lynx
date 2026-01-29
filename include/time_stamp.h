#ifndef LYNX_TIME_STAMP_H
#define LYNX_TIME_STAMP_H

#include <cstdint>
#include <string>

namespace lynx
{
static const int MicroSecond2Second = 1e6 * 1.0;
class TimeStamp
{
  private:
	int64_t micro_seconds_;

  public:
	TimeStamp() : micro_seconds_(0)
	{
	}

	explicit TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds)
	{
	}
	~TimeStamp()
	{
	}

	static TimeStamp now();
	static TimeStamp addTime(TimeStamp time_stamp, double add_seconds);
	std::string toString() const;

	int64_t microSeconds() const
	{
		return micro_seconds_;
	}

	friend std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);

	auto operator<=>(const TimeStamp& rhs) const = default;
};

std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);

} // namespace lynx

#endif