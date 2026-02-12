#include "lynx/http/http_parser.h"

using namespace lynx;

HttpParser::HttpParser() : state_(State::kStart)
{
}

HttpParser::~HttpParser()
{
}

bool HttpParser::consume(char c)
{
	switch (state_)
	{
	case State::kStart:
		if (std::isalpha(c))
		{
			req_.method += c;
			state_ = State::kMethod;
		}
		else if (c != '\r' && c != '\n')
		{
			return false;
		}
		break;

	case State::kMethod:
		if (c == ' ')
		{
			state_ = State::kPath;
		}
		else
		{
			req_.method += c;
		}
		break;

	case State::kPath:
		if (req_.path.size() > 1024)
		{
			return false;
		}

		if (c == '?')
		{
			state_ = State::kQueryKey;
		}
		else if (c == '#')
		{
			state_ = State::kFragment;
		}
		else if (c == ' ')
		{
			state_ = State::kVersion;
		}
		else
		{
			req_.path += c;
		}
		break;

	case State::kQueryKey:
		if (c == '=')
		{
			state_ = State::kQueryValue;
		}
		else if (c == ' ')
		{
			state_ = State::kVersion;
		}
		else
		{
			tmp_key_ += c;
		}
		break;

	case State::kQueryValue:
		if (c == '&')
		{
			req_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::kQueryKey;
		}
		else if (c == '#')
		{
			req_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::kFragment;
		}
		else if (c == ' ')
		{
			req_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::kVersion;
		}
		else
		{
			tmp_value_ += c;
		}
		break;

	case State::kFragment:
		if (c == ' ')
		{
			state_ = State::kVersion;
		}
		break;

	case State::kVersion:
		if (c == '\r')
		{
			state_ = State::kExpectLfAfterStatusLine;
		}
		else
		{
			req_.version += c;
		}
		break;

	case State::kExpectLfAfterStatusLine:
		if (c == '\n')
		{
			state_ = State::kHeaderKey;
		}
		else
		{
			return false;
		}
		break;

	case State::kHeaderKey:
		if (c == ':')
		{
			state_ = State::kHeaderValue;
		}
		else if (c == '\r')
		{
			state_ = State::kExpectDoubleLf;
		}
		else
		{
			tmp_key_ += std::tolower(c);
		}
		break;

	case State::kHeaderValue:
		if (c == ' ' && tmp_value_.empty())
		{
			break;
		}
		if (c == '\r')
		{

			req_.headers[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::kExpectLfAfterHeader;
		}
		else
		{
			tmp_value_ += c;
		}
		break;

	case State::kExpectLfAfterHeader:
		if (c == '\n')
		{
			state_ = State::kHeaderKey;
		}
		else
		{
			return false;
		}
		break;

	case State::kExpectDoubleLf:
		if (c == '\n')
		{
			// 检查是否有 Content-Length 决定是否需要解析 Body
			auto it = req_.headers.find("content-length");
			if (it != req_.headers.end())
			{
				body_remaining_ = std::stol(it->second);
				req_.body.reserve(body_remaining_);
				state_ =
					(body_remaining_ > 0) ? State::kBody : State::kComplete;
			}
			else
			{
				state_ = State::kComplete;
			}
		}
		else
		{
			return false;
		}
		break;

	case State::kBody:
		req_.body += c;
		if (--body_remaining_ == 0)
		{
			state_ = State::kComplete;
		}
		break;

	default:
		return false;
	}

	return true;
}