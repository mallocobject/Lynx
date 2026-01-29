#include "lynx/http/http_router.h"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"

namespace lynx
{
void HttpRouter::dispatch(const HttpRequest& req, HttpResponse* res)
{
	std::string key = req.method_raw + ':' + req.path;

	if (routes_.count(key))
	{
		routes_[key](req, res);
	}
	else
	{
		res->setStatusCode(404);
		res->setBody("<h1>404 Not Found</h1>");
		res->setContentType("text/html");
	}
}
} // namespace lynx