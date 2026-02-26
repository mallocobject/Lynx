#ifndef LYNX_HTTP_REQUEST_HPP
#define LYNX_HTTP_REQUEST_HPP

#include "lynx/base/noncopyable.hpp"
#include <cstddef>
#include <map>
#include <string>
namespace lynx
{
namespace http
{
struct Request : public base::noncopyable
{
	std::string method;
	std::string path;
	std::string version;

	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> query_params;

	std::string body;

	bool keep_alive = false;
	size_t ctx_length = 0;

	std::string header(const std::string& key) const
	{
		auto it = headers.find(key);
		return (it != headers.end()) ? it->second : "";
	}

	void clear()
	{
		method.clear();
		path.clear();
		version.clear();
		headers.clear();
		query_params.clear();
		body.clear();
		keep_alive = false;
		ctx_length = 0;
	}
};
} // namespace http
} // namespace lynx

#endif