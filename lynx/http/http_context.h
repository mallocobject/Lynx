#ifndef LYNX_HTTP_CONTEXT_H
#define LYNX_HTTP_CONTEXT_H

#include "lynx/base/noncopyable.hpp"
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