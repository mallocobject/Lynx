#ifndef LYNX_CONTEXT_HPP
#define LYNX_CONTEXT_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <strings.h>
#include <thread>

namespace lynx
{
struct Context
{
	int level{2}; // 默认 INFO 级别
	uint64_t tid;
	uint64_t timestamp; // 时间戳（微秒）

	struct Data
	{
		int line{0};
		int err{0};
		const char* short_filename{nullptr};
		const char* full_filename{nullptr};
		const char* func_name{nullptr};
	} data;

	std::string text;

	// 便利构造函数
	Context() = default;

	Context(int log_level, const char* short_file, const char* func,
			int line_num)
		: level(log_level)
	{
		tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
		data.short_filename = short_file;
		data.func_name = func;
		data.line = line_num;
	}

	// 便利接口 - 链式调用
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

	Context& withFullName(const char* long_file)
	{
		data.full_filename = long_file;
		return *this;
	}

	Context& withError(int err_code)
	{
		data.err = err_code;
		return *this;
	}

	static unsigned int getLengthWOLTT(const Context& ctx)
	{
		static const unsigned int ctx_len = (char*)&ctx.text - (char*)&ctx.data;
		return ctx_len;
	}
};
} // namespace lynx

#endif