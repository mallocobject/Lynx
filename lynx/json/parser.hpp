#ifndef LYNX_JSON_PARSER_HPP
#define LYNX_JSON_PARSER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/json/array.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/object.hpp"
#include "lynx/json/ref.hpp"
#include "lynx/json/token.hpp"
#include "lynx/json/tokenizer.hpp"
#include "lynx/json/value.hpp"
#include "lynx/logger/logger.hpp"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
namespace lynx
{
namespace json
{
class Parser : public base::noncopyable
{
  private:
	Tokenizer* tokenizer_;

  public:
	Parser(Tokenizer* tokenizer) : tokenizer_(tokenizer)
	{
	}

	Ref parse()
	{
		return Ref(parseValue());
	}

  private:
	const Token& peek() const noexcept
	{
		return tokenizer_->peek();
	}

	void consume()
	{
		tokenizer_->consume();
	}

	std::shared_ptr<Element> parseValue()
	{
		const Token& token = peek();

		switch (token.type)
		{
		case TokenType::kObjectBegin:
			return parseObject();
		case TokenType::kArrayBegin:
			return parseArray();
		case TokenType::kString:
		{
			std::string val = token.value;
			consume();
			return std::make_shared<Value>(std::move(val));
		}
		case TokenType::kInteger:
		{
			int64_t val = std::stoll(token.value);
			consume();
			return std::make_shared<Value>(std::move(val));
		}
		case TokenType::kFloat:
		{
			double val = std::stod(token.value);
			consume();
			return std::make_shared<Value>(std::move(val));
		}
		case TokenType::kBool:
		{
			bool val = token.value == "true";
			consume();
			return std::make_shared<Value>(std::move(val));
		}
		case TokenType::kNull:
			consume();
			return std::make_shared<Value>();
		case TokenType::End:
			consume();
			return nullptr;

		default:
			LOG_FATAL << "Unexpected token for a value";
			throw std::runtime_error("Unexpected token for a value");
		}
	}

	std::shared_ptr<Object> parseObject()
	{
		consume(); // for '{'

		auto obj = std::make_shared<Object>();

		if (peek().type == TokenType::kObjectEnd)
		{
			consume();
			return obj;
		}

		while (true)
		{
			const Token& key_token = peek();
			if (key_token.type != TokenType::kString)
			{
				LOG_FATAL << "Expected string key";
				throw std::runtime_error("Expected string key");
			}

			std::string key = key_token.value;
			consume();

			if (peek().type != TokenType::kColon)
			{
				LOG_FATAL << "Expected colon";
				throw std::runtime_error("Expected colon");
			}
			consume();

			// 递归解析
			obj->insert(key, parseValue());

			const Token& next_token = peek();
			if (next_token.type == TokenType::kObjectEnd)
			{
				consume();
				break;
			}
			else if (next_token.type == TokenType::kComma)
			{
				consume();
			}
			else
			{
				LOG_FATAL << "Expected comma or '}'";
				throw std::runtime_error("Expected comma or '}'");
			}
		}

		return obj;
	}

	std::shared_ptr<Array> parseArray()
	{
		consume(); // for '{'

		auto arr = std::make_shared<Array>();

		if (peek().type == TokenType::kArrayEnd)
		{
			consume();
			return arr;
		}

		while (true)
		{
			// 递归解析
			arr->append(parseValue());

			const Token& next_token = peek();
			if (next_token.type == TokenType::kArrayEnd)
			{
				consume();
				break;
			}
			else if (next_token.type == TokenType::kComma)
			{
				consume();
			}
			else
			{
				LOG_FATAL << "Expected comma or ']'";
				throw std::runtime_error("Expected comma or ']'");
			}
		}

		return arr;
	}
};
} // namespace json
} // namespace lynx

#endif