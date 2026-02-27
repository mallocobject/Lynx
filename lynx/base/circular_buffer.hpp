#ifndef LYNX_BASE_CIRCULAR_BUFFER_HPP
#define LYNX_BASE_CIRCULAR_BUFFER_HPP

#include "lynx/base/noncopyable.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <unordered_set>
#include <utility>

namespace lynx
{
namespace base
{
class SpinLock
{
	std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

  public:
	void lock()
	{
		while (flag_.test_and_set(std::memory_order_acquire))
		{
			// 忙等
		}
	}
	void unlock()
	{
		flag_.clear(std::memory_order_release);
	}
};

template <typename T, size_t N = 8> class CircularBuffer : public noncopyable
{
  public:
	using Bucket = std::unordered_set<std::shared_ptr<T>>;

  private:
	Bucket buckets_[N];
	SpinLock locks_[N];
	std::atomic<size_t> tail_{N - 1};	// 初始指向最后一个桶
	std::atomic<size_t> total_size_{0}; // 总元素个数

  public:
	CircularBuffer()
	{
	}
	~CircularBuffer()
	{
	}
	void push_back(const std::shared_ptr<T>& t)
	{
		size_t idx = tail_.load(std::memory_order_relaxed);
		locks_[idx].lock();
		buckets_[idx].insert(t);
		locks_[idx].unlock();
		total_size_.fetch_add(1, std::memory_order_relaxed);
	}

	void push_back(Bucket&& bucket)
	{
		size_t new_tail = (tail_.load(std::memory_order_relaxed) + 1) % N;
		size_t old_tail = tail_.exchange(new_tail, std::memory_order_acq_rel);
		locks_[tail_].lock();
		total_size_.fetch_sub(buckets_[tail_].size(),
							  std::memory_order_relaxed);
		buckets_[tail_] = std::move(bucket);
		locks_[tail_].unlock();
	}

	size_t size() const
	{
		return total_size_.load(std::memory_order_relaxed);
	}
};
} // namespace base
} // namespace lynx

#endif