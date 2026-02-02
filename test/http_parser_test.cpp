#include "lynx/http/http_parser.h"
#include "lynx/logger/logger.h"
#include <cassert>
#include <string>

int main()
{
	std::string raw_request =
		"POST /api/v1/user/update?id=1024&source=mobile&debug=true HTTP/1.1\r\n"
		"Host: www.example.com:8080\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n"
		"Content-Type: application/json; charset=UTF-8\r\n"
		"Content-Length: 62\r\n"
		"Cookie: session_id=abc123xyz; user_type=premium\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"{\"user_id\": 1024, \"nickname\": \"C++ Developer\", \"active\": "
		"true}";

	lynx::HttpParser parser;
	if (parser.parse(raw_request))
	{
		lynx::LOG_DEBUG << lynx::HttpParser::state2string(parser.state());
		if (parser.completed())
		{
			lynx::LOG_INFO << "Successfully parsed complex request!";
			auto req = parser.request();

			assert(req.method_raw == "POST");
			assert(req.query_params["id"] == "1024");
			assert(req.headers["Content-Length"] == "62");
		}
	}
	else
	{
		lynx::LOG_ERROR << "Parsing failed at state: "
						<< lynx::HttpParser::state2string(parser.state());
	}
}