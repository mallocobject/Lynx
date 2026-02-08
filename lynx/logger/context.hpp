#ifndef LYNX_CONTEXT_HPP
#define LYNX_CONTEXT_HPP

#include "lynx/base/time_stamp.h"
#include <cstdint>
#include <string>
#include <utility>
namespace lynx
{
struct Context
{
	TimeStamp time_stamp{0};
	uint64_t tid{0};
	int level{2};

	struct Data
	{
		int line{0};
		const char* full_name{nullptr};
		const char* short_name{nullptr};
		const char* func{nullptr};
	} data;

	std::string text;

	Context() = default;

	explicit Context(int _level, const char* _file, const char* _func,
					 int _line)
		: level(_level),
		  data({.line = _line, .short_name = _file, .func = _func})
	{
	}

	explicit Context(uint64_t _tid) : tid(_tid)
	{
	}

	Context& withText(const std::string& msg)
	{
		text = msg;
		return *this;
	}

	Context& withText(std::string&& msg)
	{
		text = std::move(msg);
		return *this;
	}

	Context& withTimeStamp(TimeStamp time)
	{
		time_stamp = time;
		return *this;
	}

	Context& withTid(uint64_t _tid)
	{
		tid = _tid;
		return *this;
	}

	Context& withLevel(int _level)
	{
		level = _level;
		return *this;
	}

	Context& WithData(Data&& _data)
	{
		data = std::move(_data);
		return *this;
	}
};
} // namespace lynx

#endif