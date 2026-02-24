#ifndef LYNX_VALUE_HPP
#define LYNX_VALUE_HPP

#include "lynx/base/alloc.hpp"
#include "lynx/json/element.hpp"
#include "lynx/logger/logger.hpp"
#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
namespace lynx
{
using value_t =
	std::variant<std::nullptr_t, bool, int64_t, double, std::string>;

template <typename T>
concept JsonValueType =
	std::same_as<std::remove_cvref_t<T>, std::nullptr_t> ||
	std::same_as<std::remove_cvref_t<T>, bool> ||
	std::integral<std::remove_cvref_t<T>> ||
	std::floating_point<std::remove_cvref_t<T>> ||
	std::convertible_to<std::remove_cvref_t<T>, std::string>;

class Value : public Element
{
  private:
	value_t val_;

  public:
	Value() : val_(nullptr)
	{
	}

	template <JsonValueType T> Value(T&& val) : val_(std::forward<T>(val))
	{
	}

	static void* operator new(size_t size)
	{
		return alloc::allocate(size);
	}

	static void operator delete(void* p, size_t size)
	{
		alloc::deallocate(p, size);
	}

	bool isValue() const noexcept override
	{
		return true;
	}

	Value* asValue() override
	{
		return this;
	}

	Element* copy() const override
	{
		return new Value(*this); // copy construction
	}

	std::string serialize() const override
	{
		if (isBool())
		{
			return asBool() ? "true" : "false";
		}
		else if (isInt())
		{
			return std::to_string(asInt());
		}
		else if (isFloat())
		{
			return std::to_string(asFloat());
		}
		else if (isStr())
		{
			return std::format("\"{}\"", asStr()); // 字符串需要加引号
		}
		else if (isNull())
		{
			return "null";
		}
		else
		{
			return "";
		}
	}

	bool isNull() const noexcept
	{
		return isT<std::nullptr_t>();
	}

	bool isBool() const noexcept
	{
		return isT<bool>();
	}

	bool isInt() const noexcept
	{
		return isT<int64_t>();
	}

	bool isFloat() const noexcept
	{
		return isT<double>();
	}

	bool isStr() const noexcept
	{
		return isT<std::string>();
	}

	bool asBool() const
	{
		if (isBool())
		{
			return asT<bool>();
		}
		LOG_FATAL << "Not bool type";
		throw std::runtime_error("Not bool type");
	}

	int64_t asInt() const
	{
		if (isInt())
		{
			return asT<int64_t>();
		}
		LOG_FATAL << "Not int type";
		throw std::runtime_error("Not int type");
	}

	double asFloat() const
	{
		if (isFloat())
		{
			return asT<double>();
		}
		LOG_FATAL << "Not float type";
		throw std::runtime_error("Not float type");
	}

	std::string asStr() const
	{
		if (isStr())
		{
			return asT<std::string>();
		}
		LOG_FATAL << "Not str type";
		throw std::runtime_error("Not str type");
	}

	std::nullptr_t asNull() const
	{
		if (isNull())
		{
			return asT<std::nullptr_t>();
		}
		LOG_FATAL << "Not null type";
		throw std::runtime_error("Not null type");
	}

  private:
	template <typename T> bool isT() const noexcept
	{
		return std::holds_alternative<T>(val_);
	}

	template <typename T> T asT() const
	{
		return std::get<T>(val_);
	}
};
} // namespace lynx

#endif