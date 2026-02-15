#ifndef LYNX_PUBLIC_ENTRY_HPP
#define LYNX_PUBLIC_ENTRY_HPP

#include "copyable.hpp"
#include <memory>
namespace lynx
{
class TcpConnection;
class Entry : public copyable
{
  private:
	std::weak_ptr<TcpConnection> weak_conn_;

  public:
	explicit Entry(const std::weak_ptr<TcpConnection>& weak_conn);
	~Entry();
};
} // namespace lynx

#endif