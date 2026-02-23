#ifndef LYNX_TOKENIZER_HPP
#define LYNX_TOKENIZER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/json/token.hpp"
#include <cctype>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
namespace lynx
{
class InputStream : public noncopyable
{
  private:
	std::string context_;
	size_t index_;

  public:
	template <typename String>
		requires std::constructible_from<std::string, String>
	InputStream(String&& context)
		: context_(std::forward<String>(context)), index_(0)
	{
	}

	InputStream(InputStream&& rhs)
		: context_(std::move(rhs.context_)), index_(rhs.index_)
	{
		rhs.index_ = 0;
	}

	char get()
	{
		return context_[index_++];
	}

	char peek() const
	{
		return context_[index_];
	}

	bool eof() const noexcept
	{
		return index_ >= context_.size();
	}
};

class Tokenizer : public noncopyable
{
  private:
	InputStream stream_;
	char ch_;
	Token token_;

  public:
	template <typename T>
		requires std::constructible_from<InputStream, T>
	Tokenizer(T&& stream) : stream_(stream)
	{
		consume();
	}

	const Token& peek() const noexcept
	{
		return token_;
	}

	void consume()
	{
		token_ = readNextToken();
	}

  private:
	Token readNextToken()
	{
		skipWihteSpace();

		if (stream_.eof())
		{
			return {TokenType::End, ""};
		}

		ch_ = stream_.peek();

		switch (ch_)
		{
		case '{':
			stream_.get();
			return {TokenType::kObjectBegin, "{"};
		case '}':
			stream_.get();
			return {TokenType::kObjectEnd, "}"};
		case '[':
			stream_.get();
			return {TokenType::kArrayBegin, "["};
		case ']':
			stream_.get();
			return {TokenType::kArrayEnd, "]"};
		case ':':
			stream_.get();
			return {TokenType::kColon, ":"};
		case ',':
			stream_.get();
			return {TokenType::kComma, ","};
		case '"':
			return parseString();
		case 't':
		case 'f':
			return parseBool();
		case 'n':
			return parseNull();

		default:
			if (isdigit(ch_) || ch_ == '-')
			{
				return parseNumber();
			}
			else
			{
				throw std::runtime_error("Unexpected character");
			}
		}
	}

	void skipWihteSpace()
	{
		while (!stream_.eof() && stream_.peek() == ' ')
		{
			stream_.get();
		}
	}

	Token parseString()
	{
		stream_.get(); // 跳过 "
		std::string str;
		while (stream_.peek() != '"')
		{
			str += stream_.get();
		}
		stream_.get(); // 跳过 "
		return {TokenType::kString, std::move(str)};
	}

	Token parseBool()
	{
		if (stream_.get() == 't' && stream_.get() == 'r' &&
			stream_.get() == 'u' && stream_.get() == 'e')
		{
			return {TokenType::kBool, "true"};
		}
		else if (stream_.get() == 'f' && stream_.get() == 'a' &&
				 stream_.get() == 'l' && stream_.get() == 's' &&
				 stream_.get() == 'e')
		{
			return {TokenType::kBool, "false"};
		}
		else
		{
			throw std::runtime_error("Unexpected boolean");
		}
	}

	Token parseNull()
	{
		if (stream_.get() == 'n' && stream_.get() == 'u' &&
			stream_.get() == 'l' && stream_.get() == 'l')
		{
			return {TokenType::kNull, "null"};
		}
		else
		{
			throw std::runtime_error("Unexpected null");
		}
	}

	Token parseNumber()
	{
		std::string num;
		bool has_dot = false;
		while (stream_.peek() == '-' || stream_.peek() == '.' ||
			   isdigit(stream_.peek()))
		{
			if (stream_.peek() == '.')
			{
				has_dot = true;
			}
			num += stream_.get();
		}

		if (has_dot)
		{
			return {TokenType::kFloat, std::move(num)};
		}
		else
		{
			return {TokenType::kInteger, std::move(num)};
		}
	}
};
} // namespace lynx

#endif