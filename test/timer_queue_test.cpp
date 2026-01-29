#include "lynx/include/event_loop.h"
#include "lynx/include/time_stamp.h"
#include "lynx/include/timer_id.hpp"
#include "lynx/include/timer_queue.h"
int main()
{
	lynx::EventLoop loop;
	lynx::TimerQueue timer_queue(&loop);

	lynx::TimerId timer1 = timer_queue.addTimer(
		lynx::TimeStamp::addTime(lynx::TimeStamp::now(), 1),
		[&]()
		{
			static int cnt = 1;
			lynx::LOG_INFO << "the task has executed " << cnt << " times.";
			if (cnt >= 5)
			{
				static bool one_run = [&]()
				{
					timer_queue.cancell(lynx::TimerId());
					return true;
				}();
			}
			if (cnt >= 10)
			{
				timer_queue.cancell(timer1);
				loop.quit();
			}
			cnt++;
		},
		2);

	loop.run();
}