#ifndef LYNX_CONFIG_H
#define LYNX_CONFIG_H

#include <cstdint>
#include <functional>
namespace lynx
{

class Flags
{
  public:
	enum Value : uint8_t
	{
		kDate = 1 << 0,
		kTime = 1 << 1,
		kFullName = 1 << 2,
		kShortName = 1 << 3,
		kLine = 1 << 4,
		kFuncName = 1 << 5,
		kThreadId = 1 << 6,
		kStdFlags = kDate | kTime | kThreadId | kShortName | kLine | kFuncName
	};

	constexpr Flags() : value_(0)
	{
	}
	constexpr Flags(Value v) : value_(v)
	{
	}

	// 位运算操作符
	constexpr Flags operator|(Flags other) const
	{
		return Flags(static_cast<Value>(value_ | other.value_));
	}

	constexpr Flags operator&(Flags other) const
	{
		return Flags(static_cast<Value>(value_ & other.value_));
	}

	constexpr Flags operator^(Flags other) const
	{
		return Flags(static_cast<Value>(value_ ^ other.value_));
	}

	constexpr Flags operator~() const
	{
		return Flags(static_cast<Value>(~value_));
	}

	constexpr Flags& operator|=(Flags other)
	{
		value_ = static_cast<Value>(value_ | other.value_);
		return *this;
	}

	constexpr Flags& operator&=(Flags other)
	{
		value_ = static_cast<Value>(value_ & other.value_);
		return *this;
	}

	constexpr Flags& operator^=(Flags other)
	{
		value_ = static_cast<Value>(value_ ^ other.value_);
		return *this;
	}

	// 转换操作符 - 支持 if (flags & Flags::kTime) 语法
	constexpr explicit operator bool() const
	{
		return value_ != 0;
	}

	// 获取值
	constexpr uint8_t value() const
	{
		return value_;
	}

  private:
	uint8_t value_;
};

struct Config
{
	using Func = std::function<void()>;

	Func log_before;
	Func log_after;
};
} // namespace lynx

#endif