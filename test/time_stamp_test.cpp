#include "lynx/include/time_stamp.h"
#include <cassert>
int main()
{
	lynx::TimeStamp t1 = lynx::TimeStamp::now();
	lynx::TimeStamp t2 = lynx::TimeStamp::addTime(t1, 5);

	assert(t1 < t2);
	assert(t2 > t1);
	assert(t1 == t1);
	assert(t1 != t2);
	assert(t1 <= t2);
	assert(t2 >= t1);
}