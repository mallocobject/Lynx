#ifndef LYNX_HTTP_PARSER_H
#define LYNX_HTTP_PARSER_H

#include "lynx/http/http_request.hpp"
#include <cassert>
#include <cstddef>
#include <string_view>
#include <utility>
namespace lynx
{
class HttpParser
{
  public:
	enum class State
	{
		START = 0,
		METHOD,
		PATH,
		QUERY_KEY,
		QUERY_VALUE,
		FRAGMENT, // skip
		VERSION,
		EXPECT_LF_AFTER_STATUS, // 等待状态行后的换行
		HEADER_KEY,
		HEADER_VALUE,
		EXPECT_LF_AFTER_HEADER, // 等待头部后的换行
		EXPECT_DOUBLE_LF,		// 等待空行（\r\n\r\n）
		BODY,
		COMPLETE,
		ERROR
	};

  private:
	State state_;

	HttpRequest request_;
	std::string tmp_key_;
	std::string tmp_value_;
	long body_remaining_;

  public:
	HttpParser() : state_(State::START), body_remaining_(0)
	{
	}

	~HttpParser()
	{
	}

	size_t bodyRemaining() const
	{
		assert(state_ == State::BODY);
		return body_remaining_;
	}

	void reset()
	{
		state_ = State::START;
		tmp_key_.clear();
		tmp_value_.clear();
		body_remaining_ = 0;

		request_.clear();
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

	bool completed() const
	{
		return state_ == State::COMPLETE;
	}

	State state() const
	{
		return state_;
	}

	void setState(State state)
	{
		state_ = state;
	}

	void appendBody(const char* data, size_t len)
	{
		request_.body.append(data, len);
		body_remaining_ -= len;
	}

	static std::string_view state2string(State state)
	{
		static const std::unordered_map<State, std::string_view> state_names = {
			{State::START, "START"},
			{State::METHOD, "METHOD"},
			{State::PATH, "PATH"},
			{State::QUERY_KEY, "QUERY_KEY"},
			{State::QUERY_VALUE, "QUERY_VALUE"},
			{State::FRAGMENT, "FRAGMENT"},
			{State::VERSION, "VERSION"},
			{State::EXPECT_LF_AFTER_STATUS, "EXPECT_LF_AFTER_STATUS"},
			{State::HEADER_KEY, "HEADER_KEY"},
			{State::HEADER_VALUE, "HEADER_VALUE"},
			{State::EXPECT_LF_AFTER_HEADER, "EXPECT_LF_AFTER_HEADER"},
			{State::EXPECT_DOUBLE_LF, "EXPECT_DOUBLE_LF"},
			{State::BODY, "BODY"},
			{State::COMPLETE, "COMPLETE"},
			{State::ERROR, "ERROR"},
		};

		auto it = state_names.find(state);
		return (it != state_names.end()) ? it->second : "UNKNOWN";
	};

	const HttpRequest& request() const
	{
		return request_;
	}

	bool consume(char c);
};
} // namespace lynx

#endif