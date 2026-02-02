#include "lynx/http/http_context.h"
#include "lynx/http/http_parser.h"
#include "lynx/http/http_request.hpp"
#include "lynx/logger/logger.h"
#include "lynx/net/buffer.h"
#include <algorithm>
#include <cstddef>
#include <memory>

namespace lynx
{
HttpContext::HttpContext() : parser_(std::make_unique<HttpParser>())
{
}

HttpContext::~HttpContext() = default;

bool HttpContext::completed() const
{
	return parser_->completed();
}

const HttpRequest& HttpContext::request() const
{
	return parser_->request();
}

void HttpContext::reset()
{
	parser_->HttpParser::reset();
}

// return true if complete or hasn't completed
bool HttpContext::parserBuffer(Buffer* buf)
{
	while (buf->readableBytes() > 0)
	{
		if (parser_->state() == HttpParser::State::BODY)
		{
			size_t n = std::min(buf->readableBytes(), parser_->bodyRemaining());
			parser_->appendBody(buf->peek(), n);

			buf->retrieve(n);

			if (parser_->bodyRemaining() == 0)
			{
				parser_->setState(HttpParser::State::COMPLETE);
				return true;
			}
			continue;
		}

		char c = *buf->peek();
		if (!parser_->consume(c))
		{
			return false;
		}

		buf->retrieve(1);

		if (parser_->completed())
		{
			return true;
		}
	}
	return true;
}
} // namespace lynx