#ifndef LYNX_FIXED_BUFFER_HPP
#define LYNX_FIXED_BUFFER_HPP

#include "lynx/base/common.hpp"
#include "lynx/logger/context.hpp"
#include <cstddef>

namespace lynx
{
template <size_t N> class FixedBuffer
{
  private:
	Context* ctxs_;
	size_t cur_index_;

  public:
	DISABLE_COPY(FixedBuffer)

	FixedBuffer() : ctxs_(new Context[N]), cur_index_(0)
	{
	}

	FixedBuffer(FixedBuffer&& rhs) noexcept
		: ctxs_(rhs.ctxs_), cur_index_(rhs.cur_index_)
	{
		rhs.ctxs_ = nullptr;
		rhs.cur_index_ = 0;
	}

	FixedBuffer& operator=(FixedBuffer&& rhs) noexcept
	{
		if (&rhs == this)
		{
			return *this;
		}

		ctxs_ = rhs.ctxs_;
		cur_index_ = rhs.cur_index_;

		rhs.ctxs_ = nullptr;
		rhs.cur_index_ = 0;

		return *this;
	}

	Context* context() const
	{
		return ctxs_;
	}

	size_t capacity() const
	{
		return N;
	}

	bool check() const
	{
		return ctxs_ != nullptr;
	}

	bool empty() const
	{
		return cur_index_ == 0;
	}

	bool full() const
	{
		return cur_index_ == N;
	}

	size_t size() const
	{
		return cur_index_;
	}

	void push(const Context& ctx)
	{
		ctxs_[cur_index_] = ctx;
		cur_index_++;
	}

	void reset()
	{
		cur_index_ = 0;
	}

	auto begin()
	{
		return ctxs_;
	}
	auto end()
	{
		return ctxs_ + cur_index_;
	}
};

} // namespace lynx

#endif