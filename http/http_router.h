#ifndef LYNX_HTTP_ROUTER_H
#define LYNX_HTTP_ROUTER_H

#include <functional>
#include <string>
#include <unordered_map>
namespace lynx
{
class HttpRequest;
class HttpResponse;
class HttpRouter
{
	using http_handler = std::function<void(const HttpRequest&, HttpResponse*)>;

  private:
	std::unordered_map<std::string, http_handler> routes_;

  public:
	void addRoute(const std::string& method, const std::string& path,
				  http_handler handler)
	{
		routes_[method + ':' + path] = std::move(handler);
	}

	void dispatch(const HttpRequest& req, HttpResponse* res);
};
} // namespace lynx

#endif