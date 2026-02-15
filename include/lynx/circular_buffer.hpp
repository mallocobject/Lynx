#ifndef LYNX_PUBLIC_CIRCULAR_BUFFER_HPP
#define LYNX_PUBLIC_CIRCULAR_BUFFER_HPP

#include "noncopyable.hpp"
#include <cstddef>
#include <memory>
#include <unordered_set>
#include <utility>

namespace lynx
{
template <typename T, size_t N = 8> class CircularBuffer : public noncopyable
{
  public:
	using Bucket = std::unordered_set<std::shared_ptr<T>>;

  private:
	Bucket buckets_[N];
	size_t tail_{7};

  public:
	CircularBuffer()
	{
	}
	~CircularBuffer()
	{
	}
	void push_back(const std::shared_ptr<T>& t)
	{
		buckets_[tail_].insert(t);
	}

	void push_back(Bucket&& bucket)
	{
		buckets_[(++tail_) %= N] = std::move(bucket);
	}

	size_t size() const
	{
		size_t sum = 0;
		for (auto& bucket : buckets_)
		{
			sum += bucket.size();
		}
		return sum;
	}
};
} // namespace lynx

#endif