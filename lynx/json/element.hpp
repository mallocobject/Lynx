#ifndef LYNX_ELEMENT_HPP
#define LYNX_ELEMENT_HPP

#include "lynx/base/copyable.hpp"
#include "lynx/logger/logger.hpp"
#include <stdexcept>
#include <string>

namespace lynx
{
class Value;
class Array;
class Object;
class Element : public copyable
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
	virtual Value* asValue()
	{
		LOG_FATAL << "Invalid value type";
		throw std::runtime_error("Invalid value type");
	}

	virtual Array* asArray()
	{
		LOG_FATAL << "Invalid array type";
		throw std::runtime_error("Invalid array type");
	}

	virtual Object* asObject()
	{
		LOG_FATAL << "Invalid object type";
		throw std::runtime_error("Invalid object type");
	}

	virtual Element* copy() const = 0;

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
} // namespace lynx

#endif