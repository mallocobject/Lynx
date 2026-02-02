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

	int64_t microseconds() const
	{
		return micro_seconds_;
	}

	friend std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);

#if __cplusplus >= 202002L
	auto operator<=>(const TimeStamp& rhs) const = default;
#else
	bool operator>(const TimeStamp& rhs) const
	{
		return micro_seconds_ > rhs.micro_seconds_;
	}

	bool operator<(const TimeStamp& rhs) const
	{
		return micro_seconds_ < rhs.micro_seconds_;
	}

	bool operator==(const TimeStamp& rhs) const
	{
		return micro_seconds_ == rhs.micro_seconds_;
	}

	bool operator<=(const TimeStamp& rhs) const
	{
		return !(*this > rhs);
	}

	bool operator>=(const TimeStamp& rhs) const
	{
		return !(*this < rhs);
	}

	bool operator!=(const TimeStamp& rhs) const
	{
		return !(*this == rhs);
	}
#endif
};

std::ostream& operator<<(std::ostream& os, const TimeStamp& ts);

} // namespace lynx

#endif