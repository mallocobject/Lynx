#ifndef LYNX_PARSER_HPP
#define LYNX_PARSER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/ref.hpp"
#include "lynx/json/token.hpp"
#include "lynx/json/tokenizer.hpp"
namespace lynx
{
class Parser : public noncopyable
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

	Element* parseValue()
	{
	}

	Object* parseObject()
	{
	}

	Array* parseArray()
	{
	}
};
} // namespace lynx

#endif