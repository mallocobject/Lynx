#include "lynx/time/timer.hpp"
#include "lynx/time/time_stamp.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <utility>

namespace lynx
{
namespace time
{
std::atomic<uint64_t> Timer::id_creator_(1);
}
}; // namespace lynx

using namespace lynx;
using namespace lynx::time;

Timer::Timer(TimeStamp expiration, std::function<void()> cb, double interval)
	: expiration_(expiration), callback_(std::move(cb)), interval_(interval),
	  repeating_(interval > 0.0), on_(false)
{
}

Timer::~Timer()
{
}