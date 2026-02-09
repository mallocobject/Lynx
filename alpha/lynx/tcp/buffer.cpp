#include "lynx/tcp/buffer.h"
#include <bits/types/struct_iovec.h>
#include <sys/uio.h>

using namespace lynx;

const int Buffer::kDataSize = 1024;
const int Buffer::kPrependSize = 8;
static const char CRLF[] = "\r\n";

Buffer::Buffer() : data_(kDataSize, 0), ridx_(kPrependSize), widx_(kPrependSize)
{
}

Buffer::~Buffer()
{
}

const char* Buffer::findCRLF(const char* start) const
{
	const char* begin = data_.data() + ridx_;
	const char* end = data_.data() + widx_;
	if (start != nullptr && (start < begin || start >= end))
	{
		return nullptr;
	}

	const char* crlf =
		std::search(start == nullptr ? begin : start, end, CRLF, CRLF + 2);
	return crlf == end ? nullptr : crlf;
}

ssize_t Buffer::readFd(int fd, int* saved_errno)
{
	thread_local char extra_buf[65536]; // 64KB
	thread_local iovec vec[2];

	const size_t writable_bytes = writableBytes();

	vec[0].iov_base = data_.data() + widx_;
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
		widx_ += n;
	}
	else
	{
		// buffer is full
		widx_ = data_.size();
		append(extra_buf, n - writable_bytes);
	}
	return n;
}

void Buffer::makeSpace(size_t len)
{
	// 前后可写区域不够
	if (writableBytes() + prependableBytes() < len + kPrependSize)
	{
		data_.resize(widx_ + len);
	}
	else
	{
		size_t readabel_bytes = readableBytes();
		std::copy(data_.data() + ridx_, data_.data() + widx_,
				  data_.data() + kPrependSize);

		ridx_ = kPrependSize;
		widx_ = ridx_ + readabel_bytes;
	}
}