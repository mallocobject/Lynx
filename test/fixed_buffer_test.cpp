#include "lynx/logger/context.hpp"
#include "lynx/logger/fixed_buffer.hpp"
#include <cassert>
#include <thread>
#include <utility>
int main()
{
	lynx::FixedBuffer<4> buf;
	assert(buf.capacity() == 4);
	assert(buf.empty());

	lynx::Context ctx = {.level = 1,
						 .tid = std::this_thread::get_id(),
						 .data =
							 {
								 .line = __LINE__,
								 .err = 0,
								 .func_name = __func__,
							 },
						 .text = "Hello world"};

	buf.push(ctx);
	assert(buf.size() == 1);
	buf.push(ctx);
	buf.push(ctx);
	buf.push(ctx);
	assert(buf.full());

	lynx::FixedBuffer<4> buf2;
	buf = std::move(buf2);

	assert(buf.capacity() == 4);
	assert(buf.size() == 0);
	assert(buf.check());
	assert(!buf2.check());
}