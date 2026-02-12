#ifndef LYNX_HTTP_PARSER_H
#define LYNX_HTTP_PARSER_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/http/http_request.hpp"
#include <cassert>
#include <cstddef>
namespace lynx
{
class HttpParser : public noncopyable
{
  public:
	enum class State
	{
		kStart,
		kMethod,
		kPath,
		kQueryKey,
		kQueryValue,
		kFragment,
		kVersion,
		kExpectLfAfterStatusLine,
		kHeaderKey,
		kHeaderValue,
		kExpectLfAfterHeader,
		kExpectDoubleLf,
		kBody,
		kComplete,
		kError
	};

  private:
	State state_;

	HttpRequest req_;
	std::string tmp_key_;
	std::string tmp_value_;
	size_t body_remaining_;

  public:
	HttpParser();
	~HttpParser();

	size_t bodyRemaining() const
	{
		assert(state_ == State::kBody);
		return body_remaining_;
	}

	void clear()
	{
		state_ = State::kStart;
		tmp_key_.clear();
		tmp_value_.clear();
		body_remaining_ = 0;

		req_.clear();
	}

	bool parse(const std::string& data)
	{
		for (char c : data)
		{
			if (!consume(c))
			{
				return false;
			}
		}
		return true;
	}

	State state() const
	{
		return state_;
	}

	void setState(State state)
	{
		state_ = state;
	}

	bool completed() const
	{
		return state_ == State::kComplete;
	}

	void appendBody(const char* data, size_t len)
	{
		req_.body.append(data, len);
		body_remaining_ -= len;
	}

	const HttpRequest& req() const
	{
		return req_;
	}

	bool consume(char c);
};
} // namespace lynx

#endif