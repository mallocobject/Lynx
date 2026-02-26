#ifndef LYNX_BASE_ENTRY_HPP
#define LYNX_BASE_ENTRY_HPP

#include "lynx/base/copyable.hpp"
#include <functional>
#include <memory>
#include <utility>
namespace lynx
{
namespace base
{
template <typename T> class Entry : public copyable
{
  private:
	std::weak_ptr<T> weak_conn_;
	std::function<void()> func_;

  public:
	explicit Entry(const std::weak_ptr<T>& weak_conn,
				   std::function<void()> func)
		: weak_conn_(weak_conn), func_(std::move(func))
	{
	}

	~Entry()
	{
		auto conn = weak_conn_.lock();
		if (conn && func_)
		{
			func_();
		}
	}
};
} // namespace base
} // namespace lynx

#endif