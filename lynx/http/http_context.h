#ifndef LYNX_HTTP_CONTEXT_H
#define LYNX_HTTP_CONTEXT_H

#include "lynx/base/common.hpp"
#include <memory>
namespace lynx
{
class HttpRequest;
class HttpParser;
class Buffer;
class HttpContext
{
  private:
	std::unique_ptr<HttpParser> parser_;

  public:
	DISABLE_COPY(HttpContext)

	HttpContext();
	~HttpContext();

	bool completed() const;

	const HttpRequest& request() const;

	void reset();

	bool parserBuffer(Buffer* buf);
};
} // namespace lynx

#endif