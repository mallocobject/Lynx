#include "lynx/http/http_response.h"
#include <format>
#include <string>

namespace lynx
{
std::string HttpResponse::toString() const
{
	std::string result;

	// result += std::format("{} {} {}\r\n", version_, status_code_, status_msg_);
	result = std::format_to(result, "{} {} {}\r\n", version_, status_code_,
						 status_msg_);

	for (auto& header : headers_)
	{
		// result += std::format("{}: {}\r\n", header.first, header.second);
		result = std::format_to(result, "{}: {}\r\n", header.first,
								header.second);
	}

	// result += std::format("\r\n{}", body_);
	result = std::format_to(result, "\r\n{}", body_);

	return result;
}
} // namespace lynx