#ifndef LYNX_TOKEN_HPP
#define LYNX_TOKEN_HPP

#include <string>
namespace lynx
{
enum class TokenType
{
	kObjectBegin, // '{'
	kObjectEnd,	  // '}'
	kArrayBegin,  // '['
	kArrayEnd,	  // ']'
	kColon,		  // ':'
	kComma,		  // ','
	kString,	  // "string"
	kInteger,	  // 123
	kFloat,		  // 12.72
	kBool,		  // true of false
	kNull,		  // null
	End			  // end of input
};

struct Token
{
	TokenType type;
	std::string value;
};
} // namespace lynx

#endif