#include "lynx/http/context.hpp"
#include "lynx/http/parser.hpp"
#include "lynx/tcp/buffer.hpp"
#include <memory>

using namespace lynx;
using namespace lynx::http;

Context::Context()
{
	parser_ = std::make_unique<Parser>();
}

Context::~Context()
{
}

bool Context::completed() const
{
	return parser_->completed();
}

const Request& Context::req() const
{
	return parser_->req();
}

void Context::clear()
{
	parser_->clear();
}

bool Context::parser(tcp::Buffer* buf)
{
	while (buf->readableBytes() > 0)
	{
		if (parser_->state() == Parser::State::kBody)
		{
			size_t n = std::min(buf->readableBytes(), parser_->bodyRemaining());
			parser_->appendBody(buf->peek(), n);

			buf->retrieve(n);

			if (parser_->bodyRemaining() == 0)
			{
				parser_->setState(Parser::State::kComplete);
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