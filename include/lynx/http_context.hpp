#ifndef LYNX_PUBLIC_HTTP_CONTEXT_HPP
#define LYNX_PUBLIC_HTTP_CONTEXT_HPP

#include "noncopyable.hpp"
#include <memory>
namespace lynx
{
class HttpRequest;
class HttpParser;
class Buffer;
class HttpContext : public noncopyable
{
  private:
	std::unique_ptr<HttpParser> parser_;

  public:
	HttpContext();
	~HttpContext();

	bool completed() const;
	const HttpRequest& req() const;
	void clear();
	bool parser(Buffer* buf);
};
} // namespace lynx

#endif