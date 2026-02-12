#ifndef LYNX_CURRENT_THREAD_HPP
#define LYNX_CURRENT_THREAD_HPP

#include <cstdint>
#include <functional>
#include <sstream>
#include <thread>
namespace lynx
{
namespace CurrentThread
{
inline uint64_t tid()
{
	thread_local uint64_t cached_tid =
		std::hash<std::thread::id>{}(std::this_thread::get_id());
	return cached_tid;
}

inline std::ostringstream& oss()
{
	thread_local std::ostringstream oss;
	static bool initialed = []()
	{
		oss.str("");
		oss.clear();
		return true;
	}();

	return oss;
}
} // namespace CurrentThread
} // namespace lynx

#endif