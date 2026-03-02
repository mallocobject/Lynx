#include <cassert>
#include <lynx/lynx.hpp>

using namespace lynx;
using namespace lynx::time;

int main()
{
	tcp::EventLoop loop;
	auto tid = loop.runAfter(5, [] { LOG_INFO << "Timer runs"; });
	loop.runEvery(1, [&tid] { LOG_INFO << tid.isAlive(); });
	loop.run();
}