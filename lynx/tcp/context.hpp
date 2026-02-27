#ifndef LYNX_TCP_CONTEXT_HPP
#define LYNX_TCP_CONTEXT_HPP

#include "lynx/base/entry.hpp"
#include "lynx/http/session.hpp"
#include "lynx/tcp/connection.hpp"
#include <memory>

namespace lynx
{
namespace tcp
{
struct Context
{
	std::shared_ptr<http::Session> session_;
	std::weak_ptr<base::Entry<tcp::Connection>> entry_;
};
} // namespace tcp
} // namespace lynx

#endif