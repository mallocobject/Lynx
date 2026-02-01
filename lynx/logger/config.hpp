#ifndef LYNX_CONFIG_H
#define LYNX_CONFIG_H

#include <cstdint>
#include <functional>
namespace lynx
{

enum class Flags : uint8_t
{
	kDate = 1 << 0,
	kTime = 1 << 1,
	kFullName = 1 << 2,
	kShortName = 1 << 3,
	kLine = 1 << 4,
	kFuncName = 1 << 5,
	kThreadId = 1 << 6,
	kStdFlags = kDate | kTime | kShortName | kLine | kFuncName
};

struct Config
{
	using Func = std::function<void()>;

	Func log_before;
	Func log_after;
};
} // namespace lynx

#endif