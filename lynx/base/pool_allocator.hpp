#ifndef LYNX_BASE_POOL_ALLOCATOR_HPP
#define LYNX_BASE_POOL_ALLOCATOR_HPP

#include "lynx/base/alloc.hpp"
#include <cstddef>
namespace lynx
{
namespace base
{
template <typename T> struct PoolAllocator
{
	using value_type = T;

	PoolAllocator() = default;

	template <typename U> PoolAllocator(const PoolAllocator<U>&) noexcept
	{
	}

	T* allocate(size_t n)
	{
		void* ptr = base::alloc::allocate(n * sizeof(T));
		if (!ptr)
		{
			throw std::bad_alloc();
		}
		return reinterpret_cast<T*>(ptr);
	}

	void deallocate(T* p, size_t n) noexcept
	{
		alloc::deallocate(p, n * sizeof(T));
	}

	template <typename U> struct rebind
	{
		using other = PoolAllocator<U>;
	};
};

template <typename T, typename U>
bool operator==(const PoolAllocator<T>&, const PoolAllocator<U>&) noexcept
{
	return true;
}
template <typename T, typename U>
bool operator!=(const PoolAllocator<T>&, const PoolAllocator<U>&) noexcept
{
	return false;
}
} // namespace base
} // namespace lynx

#endif