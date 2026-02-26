#ifndef LYNX_HTTP_CONTEXT_HPP
#define LYNX_HTTP_CONTEXT_HPP

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
class Context : public base::noncopyable
{
  private:
	std::unique_ptr<Parser> parser_;

  public:
	Context();
	~Context();

	bool completed() const;
	const Request& req() const;
	void clear();
	bool parser(tcp::Buffer* buf);
};
} // namespace http
} // namespace lynx

#endif