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
	int read_index_;
	int write_index_;

  public:
	DISABLE_COPY_AND_MOVE(Buffer)

	Buffer();

	std::size_t size() const
	{
		return data_.size();
	}

	void input(const std::string& str);
	std::string output();
};
} // namespace lynx

#endif