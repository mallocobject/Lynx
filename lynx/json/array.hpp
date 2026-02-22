#ifndef LYNX_ARRAY_HPP
#define LYNX_ARRAY_HPP

#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <cstddef>
#include <vector>
namespace lynx
{
using array_t = std::vector<Element*>;

class Array : public Element
{
  private:
	array_t arr_;

  public:
	Array()
	{
	}
	~Array() override
	{
		clear();
	}

	bool isArray() const noexcept override
	{
		return true;
	}

	Array* asArray() override
	{
		return this;
	}

	Element* copy() const override
	{
		Array* new_arr = new Array;
		for (const auto& e : arr_)
		{
			new_arr->appendRawPtr(e->copy());
		}

		return new_arr;
	}

	template <JsonValueType T> void append(T&& val)
	{
		appendRawPtr(new Value(val));
	}

	Element* at(size_t index)
	{
		return arr_.at(index);
	}

	Element* operator[](size_t index)
	{
		return arr_.at(index);
	}

  private:
	void clear() override
	{
		for (const auto& e : arr_)
		{
			if (e)
			{
				delete e;
			}
		}
		arr_.clear();
	}

	void appendRawPtr(Element* child)
	{
		arr_.push_back(child);
	}
};
} // namespace lynx

#endif