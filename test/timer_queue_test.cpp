
#include "lynx/base/time_stamp.h"
#include "lynx/base/timer_id.hpp"
#include "lynx/base/timer_queue.h"
#include "lynx/logger/logger.h"
#include "lynx/tcp/event_loop.h"
#include <cassert>
using namespace lynx;
int main()
{
	EventLoop loop;
	TimerQueue tq(&loop);

	TimerId tid = tq.addTimer(
		TimeStamp::now(),
		[&tq, &tid]()
		{
			static int cnt = 0;
			LOG_INFO << "timer works now";
			if (++cnt >= 5)
			{
				tq.cancell(tid);
				assert(tq.size() == 1);
			}
		},
		2);
	assert(tq.size() == 1);

	TimerId tid2 = tq.addTimer(TimeStamp::now(), []() {}, 1);
	assert(tq.size() == 2);
	loop.run();
}