#include "lynx/time/timer.h"
#include "lynx/time/time_stamp.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <utility>

namespace lynx
{
std::atomic<int64_t> Timer::seq_creator_(0);
};

using namespace lynx;

Timer::Timer(TimeStamp expiration, std::function<void()> cb, double interval)
	: expiration_(expiration), callback_(std::move(cb)), interval_(interval),
	  repeating_(interval > 0.0), on_(false)
{
	seq_ = seq_creator_.fetch_add(1, std::memory_order_acq_rel);
}

Timer::~Timer()
{
}