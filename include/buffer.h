#ifndef LYNX_BUFFER_H
#define LYNX_BUFFER_H

#include "lynx/include/common.h"
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace lynx
{
class Buffer
{
  private:
	std::vector<char> data_;
	size_t read_index_;
	size_t write_index_;

  public:
	DISABLE_COPY_AND_MOVE(Buffer)

	Buffer();
};
} // namespace lynx

#endif