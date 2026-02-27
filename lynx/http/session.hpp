#ifndef LYNX_HTTP_SESSION_HPP
#define LYNX_HTTP_SESSION_HPP

#include "lynx/base/noncopyable.hpp"
#include <memory>
namespace lynx
{

namespace tcp
{
class Buffer;
}
namespace http
{
class Request;
class Parser;
class Session : public base::noncopyable
{
  private:
	std::unique_ptr<Parser> parser_;

  public:
	Session();
	~Session();

	bool completed() const;
	const Request& req() const;
	void clear();
	bool parser(tcp::Buffer* buf);
};
} // namespace http
} // namespace lynx

#endif