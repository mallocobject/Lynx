#include "lynx/include/buffer.h"
#include <bits/types/struct_iovec.h>
#include <cstddef>
#include <sys/types.h>
#include <sys/uio.h>

namespace lynx
{
Buffer::Buffer()
	: data_(DATA_SIZE, 0), read_index_(PREPEND_SIZE), write_index_(PREPEND_SIZE)
{
}

ssize_t Buffer::readFd(int fd, int* saved_errno)
{
	char extra_buf[65536]; // 64KB
	iovec vec[2];
	const size_t writable_bytes = writableBytes();

	vec[0].iov_base = data_.data() + write_index_;
	vec[0].iov_len = writable_bytes;

	vec[1].iov_base = extra_buf;
	vec[1].iov_len = sizeof(extra_buf);

	const ssize_t n = ::readv(fd, vec, 2); // LT 触发

	if (n < 0)
	{
		*saved_errno = errno; // move to upper struct
	}
	else if (static_cast<size_t>(n) <= writable_bytes)
	{
		write_index_ += n;
	}
	else
	{
		// buffer is full
		write_index_ = data_.size();
		append(extra_buf, n - writable_bytes);
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	// 前后可写区域不够
	if (writableBytes() + prependableBytes() < len + PREPEND_SIZE)
	{
		data_.resize(write_index_ + len);
	}
	else
	{
		size_t readabel_bytes = readableBytes();
		std::copy(data_.data() + read_index_, data_.data() + write_index_,
				  data_.data() + PREPEND_SIZE);

		read_index_ = PREPEND_SIZE;
		write_index_ = read_index_ + readabel_bytes;
	}
}

} // namespace lynx