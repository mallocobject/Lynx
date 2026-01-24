#include "lynx/include/buffer.h"
#include <string>

#define DATA_SIZE 1024 * 8
#define PREPPEND_SIZE 8

namespace lynx
{
Buffer::Buffer()
	: data_(DATA_SIZE, 0), read_index_(PREPPEND_SIZE),
	  write_index_(PREPPEND_SIZE)
{
}

} // namespace lynx