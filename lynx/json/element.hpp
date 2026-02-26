#ifndef LYNX_JSON_ELEMENT_HPP
#define LYNX_JSON_ELEMENT_HPP

#include "lynx/base/copyable.hpp"
#include "lynx/logger/logger.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace lynx
{
namespace json
{

class Value;
class Array;
class Object;
class Element : public base::copyable,
				public std::enable_shared_from_this<Element>
{
  public:
	virtual ~Element()
	{
	}

	virtual bool isValue() const noexcept
	{
		return false;
	}

	virtual bool isArray() const noexcept
	{
		return false;
	}

	virtual bool isObject() const noexcept
	{
		return false;
	}

	// fail-fast
	virtual std::shared_ptr<Value> asValue()
	{
		LOG_FATAL << "Invalid value type";
		throw std::runtime_error("Invalid value type");
	}

	virtual std::shared_ptr<Array> asArray()
	{
		LOG_FATAL << "Invalid array type";
		throw std::runtime_error("Invalid array type");
	}

	virtual std::shared_ptr<Object> asObject()
	{
		LOG_FATAL << "Invalid object type";
		throw std::runtime_error("Invalid object type");
	}

	virtual std::shared_ptr<Element> copy() const = 0;

	virtual std::string serialize() const
	{
		return "";
	}

	virtual void clear()
	{
	}

	virtual bool operator==(const Element& rhs) const noexcept
	{
		return false;
	}

	virtual bool operator!=(const Element& rhs) const noexcept
	{
		return true;
	}
};
} // namespace json
} // namespace lynx

#endif