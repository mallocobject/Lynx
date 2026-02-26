#ifndef LYNX_LOGGER_CONTEXT_HPP
#define LYNX_LOGGER_CONTEXT_HPP

#include "lynx/time/time_stamp.hpp"
#include <cstdint>
#include <string>
#include <utility>
namespace lynx
{
namespace logger
{
struct Context
{
	time::TimeStamp time_stamp{0};
	uint64_t tid{0};
	uint8_t level{2};

	struct Data
	{
		int line{0};
		const char* full_name{nullptr};
		const char* short_name{nullptr};
		const char* func{nullptr};
	} data;

	std::string text;

	Context() = default;

	explicit Context(uint8_t _level, const char* _file, const char* _func,
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

	Context& withTimeStamp(time::TimeStamp time)
	{
		time_stamp = time;
		return *this;
	}

	Context& withTid(uint64_t _tid)
	{
		tid = _tid;
		return *this;
	}

	Context& withLevel(uint8_t _level)
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
} // namespace logger
} // namespace lynx

#endif