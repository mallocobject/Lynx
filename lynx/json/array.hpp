#ifndef LYNX_JSON_ARRAY_HPP
#define LYNX_JSON_ARRAY_HPP

#include "lynx/base/alloc.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace lynx
{
namespace json
{
using array_t = std::vector<std::shared_ptr<Element>>;

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
	}

	// static void* operator new(size_t size)
	// {
	// 	return base::alloc::allocate(size);
	// }

	// static void operator delete(void* p, size_t size)
	// {
	// 	base::alloc::deallocate(p, size);
	// }

	bool isArray() const noexcept override
	{
		return true;
	}

	std::shared_ptr<Array> asArray() override
	{
		return std::dynamic_pointer_cast<Array>(shared_from_this());
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

	std::shared_ptr<Element> copy() const override
	{
		auto new_arr = std::make_shared<Array>();
		for (const auto& v : arr_)
		{
			new_arr->append(v->copy());
		}

		return new_arr;
	}

	template <JsonValueType T> void append(T&& val)
	{
		append(std::make_shared<Value>(std::forward<T>(val)));
	}

	void append(std::shared_ptr<Element> child)
	{
		arr_.push_back(std::move(child));
	}

	std::shared_ptr<Element>& at(size_t index)
	{
		return arr_.at(index);
	}

	std::shared_ptr<Element>& operator[](size_t index)
	{
		return arr_.at(index);
	}

	const std::shared_ptr<Element>& operator[](size_t index) const
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
};
} // namespace json
} // namespace lynx

#endif