#include "lynx/http/http_parser.h"
#include "lynx/include/logger.hpp"
#include <cctype>
#include <string>

namespace lynx
{
bool HttpParser::consume(char c)
{
	switch (state_)
	{
	case State::START:
		if (std::isalpha(c))
		{
			request_.method_raw += c;
			state_ = State::METHOD;
		}
		else if (c != '\r' && c != '\n')
		{
			return false;
		}
		break;

	case State::METHOD:
		if (c == ' ')
		{
			state_ = State::PATH;
		}
		else
		{
			request_.method_raw += c;
		}
		break;

	case State::PATH:
		if (request_.path.size() > 8192)
		{
			return false;
		}

		if (c == '?')
		{
			state_ = State::QUERY_KEY;
		}
		else if (c == '#')
		{
			state_ = State::FRAGMENT;
		}
		else if (c == ' ')
		{
			state_ = State::VERSION;
		}
		else
		{
			request_.path += c;
		}
		break;

	case State::QUERY_KEY:
		if (c == '=')
		{
			state_ = State::QUERY_VALUE;
		}
		else if (c == ' ')
		{
			state_ = State::VERSION;
		}
		else
		{
			tmp_key_ += c;
		}
		break;

	case State::QUERY_VALUE:
		if (c == '&')
		{
			request_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::QUERY_KEY;
		}
		else if (c == '#')
		{
			request_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::FRAGMENT;
		}
		else if (c == ' ')
		{
			request_.query_params[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::VERSION;
		}
		else
		{
			tmp_value_ += c;
		}
		break;

	case State::FRAGMENT:
		if (c == ' ')
		{
			state_ = State::VERSION;
		}
		break;

	case State::VERSION:
		if (c == '\r')
		{
			state_ = State::EXPECT_LF_AFTER_STATUS;
		}
		else
		{
			request_.version += c;
		}
		break;

	case State::EXPECT_LF_AFTER_STATUS:
		if (c == '\n')
		{
			state_ = State::HEADER_KEY;
		}
		else
		{
			return false;
		}
		break;

	case State::HEADER_KEY:
		if (c == ':')
		{
			state_ = State::HEADER_VALUE;
		}
		else if (c == '\r')
		{
			state_ = State::EXPECT_DOUBLE_LF;
		}
		else
		{
			tmp_key_ += std::tolower(c);
		}
		break;

	case State::HEADER_VALUE:
		if (c == ' ' && tmp_value_.empty())
		{
			break;
		}
		if (c == '\r')
		{

			request_.headers[tmp_key_] = tmp_value_;
			tmp_key_.clear();
			tmp_value_.clear();
			state_ = State::EXPECT_LF_AFTER_HEADER;
		}
		else
		{
			tmp_value_ += c;
		}
		break;

	case State::EXPECT_LF_AFTER_HEADER:
		if (c == '\n')
		{
			state_ = State::HEADER_KEY;
		}
		else
		{
			return false;
		}
		break;

	case State::EXPECT_DOUBLE_LF:
		if (c == '\n')
		{
			// 检查是否有 Content-Length 决定是否需要解析 Body
			auto it = request_.headers.find("content-length");
			if (it != request_.headers.end())
			{
				body_remaining_ = std::stol(it->second);
				request_.body.reserve(body_remaining_);
				state_ = (body_remaining_ > 0) ? State::BODY : State::COMPLETE;
			}
			else
			{
				state_ = State::COMPLETE;
			}
		}
		else
		{
			return false;
		}
		break;

	case State::BODY:
		request_.body += c;
		if (--body_remaining_ == 0)
		{
			state_ = State::COMPLETE;
		}
		break;

	default:
		return false;
	}

	return true;
}
} // namespace lynx