#include "lynx/http/http_context.h"
#include "lynx/http/http_parser.h"
#include "lynx/tcp/buffer.h"
#include <memory>

using namespace lynx;

HttpContext::HttpContext()
{
	parser_ = std::make_unique<HttpParser>();
}

HttpContext::~HttpContext()
{
}

bool HttpContext::completed() const
{
	return parser_->completed();
}

const HttpRequest& HttpContext::req() const
{
	return parser_->req();
}

void HttpContext::clear()
{
	parser_->clear();
}

bool HttpContext::parser(Buffer* buf)
{
	while (buf->readableBytes() > 0)
	{
		if (parser_->state() == HttpParser::State::kBody)
		{
			size_t n = std::min(buf->readableBytes(), parser_->bodyRemaining());
			parser_->appendBody(buf->peek(), n);

			buf->retrieve(n);

			if (parser_->bodyRemaining() == 0)
			{
				parser_->setState(HttpParser::State::kComplete);
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