#ifndef LYNX_BASE_CURRENT_THREAD_HPP
#define LYNX_BASE_CURRENT_THREAD_HPP

#include <cstdint>
#include <functional>
#include <thread>
namespace lynx
{
namespace base
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
		str.reserve(128);
		return true;
	}();

	return str;
}
} // namespace CurrentThread
} // namespace base
} // namespace lynx

#endif