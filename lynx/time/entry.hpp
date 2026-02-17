#ifndef LYNX_ENTRY_HPP
#define LYNX_ENTRY_HPP

#include "lynx/base/copyable.hpp"
#include <memory>
namespace lynx
{
template <typename T> class Entry : public copyable
{
  private:
	std::weak_ptr<T> weak_conn_;

  public:
	explicit Entry(const std::weak_ptr<T>& weak_conn) : weak_conn_(weak_conn)
	{
	}

	~Entry()
	{
		auto conn = weak_conn_.lock();
		if (conn)
		{
			conn->shutdown();
		}
	}
};
} // namespace lynx

#endif