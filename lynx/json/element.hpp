#ifndef LYNX_ELEMENT_HPP
#define LYNX_ELEMENT_HPP

#include "lynx/base/copyable.hpp"
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
		throw std::runtime_error("Invalid base type");
	}

	virtual Array* asArray()
	{
		throw std::runtime_error("Invalid base type");
	}

	virtual Object* asObject()
	{
		throw std::runtime_error("Invalid base type");
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