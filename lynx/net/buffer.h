#ifndef LYNX_BUFFER_H
#define LYNX_BUFFER_H

#include "lynx/base/common.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace lynx
{

#define DATA_SIZE 1024
#define PREPEND_SIZE 8

class Buffer
{
  private:
	std::vector<char> data_;
	size_t read_index_;
	size_t write_index_;

  public:
	DISABLE_COPY(Buffer)

	Buffer();

	size_t readableBytes() const
	{
		assert(write_index_ >= read_index_);
		return write_index_ - read_index_;
	}

	size_t writableBytes() const
	{
		assert(data_.size() >= write_index_);
		return data_.size() - write_index_;
	}

	size_t prependableBytes() const
	{
		// assert(read_index_ >= 0);
		return read_index_;
	}

	// output / read operations

	const char* peek() const
	{
		return data_.data() + read_index_;
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if (len < readableBytes())
		{
			read_index_ += len;
		}
		else
		{
			retrieveAll();
		}
	}

	std::string retrieveString(size_t len)
	{
		assert(len <= readableBytes());
		std::string result(peek(), len);
		retrieve(len);
		return result;
	}

	// input / write operations
	void append(const char* data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, data_.data() + write_index_);
		write_index_ += len;
	}

	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		read_index_ -= len;
		const char* d = reinterpret_cast<const char*>(data);
		std::copy(d, d + len, data_.data() + read_index_);
	}

	ssize_t readFd(int fd, int* saved_errno);

	void shrink(size_t reserve)
	{
		Buffer other;
		other.ensureWritableBytes(readableBytes() + reserve);
		other.append(peek(), readableBytes());
		swap(other);
	}

	void swap(Buffer& rhs)
	{
		data_.swap(rhs.data_);
		std::swap(read_index_, rhs.read_index_);
		std::swap(write_index_, rhs.write_index_);
	}

	// auxiliary functions

	void appendInt32(int32_t x)
	{
		int32_t be32 = htonl(x); // 转为网络字节序
		append(reinterpret_cast<const char*>(&be32), sizeof(be32));
	}

	int32_t peekInt32() const
	{
		assert(readableBytes() >= sizeof(int32_t));
		int32_t be32 = 0;
		::memcpy(&be32, peek(), sizeof(be32));
		return ntohl(be32); // 从网络字节序转回主机字节序
	}

	int32_t retrieveInt32()
	{
		int32_t result = peekInt32();
		retrieve(sizeof(int32_t));
		return result;
	}

	void prependInt32(int32_t x)
	{
		int32_t be32 = htonl(x);
		prepend(&be32, sizeof(be32));
	}

	const char* findCRLF(const char* start = nullptr) const;

  private:
	void retrieveAll()
	{
		read_index_ = PREPEND_SIZE;
		write_index_ = PREPEND_SIZE;
	}

	void ensureWritableBytes(size_t len)
	{
		if (len > writableBytes())
		{
			makeSpace(len);
		}
	}

	void makeSpace(size_t len);
};
} // namespace lynx

#endif