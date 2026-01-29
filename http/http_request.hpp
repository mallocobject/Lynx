#ifndef LYNX_HTTP_REQUEST_HPP
#define LYNX_HTTP_REQUEST_HPP

#include <cstddef>
#include <string>
#include <unordered_map>

namespace lynx
{
enum class HttpMethod
{
	GET,
	POST,
	UNKNOWN
};

struct HttpRequest
{
	// HttpMethod method_ = HttpMethod::UNKNOWN;
	std::string method_raw;
	std::string path;
	std::string version;

	std::unordered_map<std::string, std::string> headers;
	std::unordered_map<std::string, std::string> query_params;

	std::string body;

	bool keep_alive = false;
	size_t content_length = 0;

	std::string header(const std::string& key) const
	{
		auto it = headers.find(key);
		return (it != headers.end()) ? it->second : "";
	}

	void clear()
	{
		method_raw.clear();
		path.clear();
		headers.clear();
		query_params.clear();
		body.clear();
		keep_alive = false;
		content_length = 0;
	}
};
} // namespace lynx

#endif