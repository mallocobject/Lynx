#ifndef LYNX_BUFFER_H
#define LYNX_BUFFER_H

#include "lynx/base/noncopyable.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
namespace lynx
{
class Buffer : noncopyable
{
  private:
	std::vector<char> data_;
	size_t ridx_;
	size_t widx_;
	static const int kDataSize;
	static const int kPrependSize;

  public:
	Buffer();
	~Buffer();

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
} // namespace lynx

#endif