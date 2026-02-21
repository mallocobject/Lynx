#ifndef LYNX_CURRENT_THREAD_HPP
#define LYNX_CURRENT_THREAD_HPP

#include <cstdint>
#include <functional>
#include <string>
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

inline std::string& str()
{
	thread_local std::string str;
	static bool initialed = []()
	{
		str.clear();
		return true;
	}();

	return str;
}
} // namespace CurrentThread
} // namespace lynx

#endif