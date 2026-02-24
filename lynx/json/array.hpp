#ifndef LYNX_ARRAY_HPP
#define LYNX_ARRAY_HPP

#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <cstddef>
#include <format>
#include <iterator>
#include <string>
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

	static void* operator new(size_t size)
	{
		return alloc::allocate(size);
	}

	static void operator delete(void* p, size_t size)
	{
		alloc::deallocate(p, size);
	}

	bool isArray() const noexcept override
	{
		return true;
	}

	Array* asArray() override
	{
		return this;
	}

	std::string serialize() const override
	{
		std::string result = "[";
		bool first = true;
		for (const auto& v : arr_)
		{
			if (!first)
			{
				result += ',';
			}
			result += v->serialize(); // 多态
			first = false;
		}
		result += ']';
		return result;
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

	void appendRawPtr(Element* child)
	{
		arr_.push_back(child);
	}

	Element*& at(size_t index)
	{
		return arr_.at(index);
	}

	Element*& operator[](size_t index)
	{
		return arr_[index];
	}

	size_t size() const noexcept
	{
		return arr_.size();
	}

	array_t::const_iterator begin() const
	{
		return arr_.begin();
	}

	array_t::const_iterator end() const
	{
		return arr_.end();
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
};
} // namespace lynx

#endif