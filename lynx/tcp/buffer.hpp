#ifndef LYNX_TCP_BUFFER_HPP
#define LYNX_TCP_BUFFER_HPP

#include "lynx/base/noncopyable.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <vector>
namespace lynx
{
namespace tcp
{
class Buffer : base::noncopyable
{
  private:
	std::vector<char> data_;
	size_t ridx_;
	size_t widx_;
	static const int kDataSize;
	static const int kPrependSize;
	bool enabled_read_;

  public:
	Buffer();
	~Buffer();

	bool reading() const
	{
		return enabled_read_;
	}

	void setReading(bool on = true)
	{
		enabled_read_ = on;
	}

	size_t readableBytes() const
	{
		assert(widx_ >= ridx_);
		return widx_ - ridx_;
	}

	size_t writableBytes() const
	{
		assert(data_.size() >= widx_);
		return data_.size() - widx_;
	}

	size_t prependableBytes() const
	{
		return ridx_;
	}

	const char* peek() const
	{
		return data_.data() + ridx_;
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if (len < readableBytes())
		{
			ridx_ += len;
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

	void append(const char* data, size_t len)
	{
		if (!enabled_read_)
		{
			return;
		}

		ensureWritableBytes(len);
		std::copy(data, data + len, data_.data() + widx_);
		widx_ += len;
	}

	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		ridx_ -= len;
		const char* d = reinterpret_cast<const char*>(data);
		std::copy(d, d + len, data_.data() + ridx_);
	}

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
		std::swap(ridx_, rhs.ridx_);
		std::swap(widx_, rhs.widx_);
	}

	// auxiliary functions

	void appendInt32(int32_t x)
	{
		int32_t be32 = ::htonl(x); // 转为网络字节序
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
		int32_t be32 = ::htonl(x);
		prepend(&be32, sizeof(be32));
	}

	const char* findCRLF(const char* start = nullptr) const;

	ssize_t readFd(int fd, int* saved_errno);

  private:
	void retrieveAll()
	{
		ridx_ = kPrependSize;
		widx_ = kPrependSize;
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
} // namespace tcp
} // namespace lynx

#endif