#ifndef LYNX_CONTEXT_HPP
#define LYNX_CONTEXT_HPP

#include <string>
#include <strings.h>
#include <thread>

namespace lynx
{
struct Context
{
	int level;
	std::thread::id tid;

	struct Data
	{
		int line{0};
		int err{0};
		const char* short_filename{nullptr};
		const char* long_filename{nullptr};
		const char* func_name{nullptr};
	} data;

	std::string text;

	static unsigned int getLengthWOLTT(const Context& ctx)
	{
		static const unsigned int ctx_len = (char*)&ctx.text - (char*)&ctx.data;
		return ctx_len;
	}
};
} // namespace lynx

#endif