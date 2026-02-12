#include "lynx/http/http_response.h"
#include <format>

using namespace lynx;

HttpResponse::HttpResponse() : version_("HTTP/1.1")
{
}
HttpResponse::~HttpResponse()
{
}

std::string HttpResponse::toFormattedString() const
{
	std::string result;

	auto out = std::back_inserter(result);

	// 第一行
	std::format_to(out, "{} {} {}\r\n", version_, status_code_, status_msg_);

	// 头部
	for (const auto& header : headers_)
	{
		std::format_to(out, "{}: {}\r\n", header.first, header.second);
	}

	// 空行和正文
	std::format_to(out, "\r\n{}", body_);

	return result;
}