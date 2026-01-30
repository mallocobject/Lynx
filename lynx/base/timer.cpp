#include "lynx/base/timer.h"
#include <atomic>
#include <cstdint>
#include <utility>

namespace lynx
{
std::atomic<int64_t> Timer::seq_creator_(1);

Timer::Timer(TimeStamp expiration, std::function<void()> cb, double interval)
	: expiration_(expiration), callback_(std::move(cb)), interval_(interval),
	  repeat_(interval > 0.0), sequence_(seq_creator_++)
{
}
} // namespace lynx