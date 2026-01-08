#include "lynx/include/buffer.h"
#include <string>

#define DATA_SIZE 1024 * 8

namespace lynx
{
Buffer::Buffer() : data_(DATA_SIZE, 0), read_index_(0), write_index_(0)
{
}

void Buffer::input(const std::string& str)
{
	for (int i = write_index_; i < str.size(); i++)
	{
		data_[i] = str[i - write_index_];
	}
}

std::string Buffer::output()
{
	return std::string(data_.begin() + read_index_,
					   data_.begin() + write_index_);
}

} // namespace lynx