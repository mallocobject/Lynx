#ifndef LYNX_JSON_OBJECT_HPP
#define LYNX_JSON_OBJECT_HPP

#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <cstddef>
#include <format>
#include <map>
#include <string>
namespace lynx
{
namespace json
{
using object_t = std::map<std::string, Element*>;

class Object : public Element
{
  private:
	object_t obj_;

  public:
	Object()
	{
	}
	~Object() override
	{
		clear();
	}

	static void* operator new(size_t size)
	{
		return base::alloc::allocate(size);
	}

	static void operator delete(void* p, size_t size)
	{
		base::alloc::deallocate(p, size);
	}

	bool isObject() const noexcept override
	{
		return true;
	}

	Object* asObject() override
	{
		return this;
	}

	std::string serialize() const override
	{
		std::string result = "{";
		bool first = true;
		for (const auto& kv : obj_)
		{
			if (!first)
			{
				result += ',';
			}
			result +=
				std::format("\"{}\":{}", kv.first, kv.second->serialize());
			first = false;
		}
		result += '}';
		return result;
	}

	Element* copy() const override
	{
		Object* new_obj = new Object;
		for (const auto& [key, val] : obj_)
		{
			new_obj->insertRawPtr(key, val);
		}

		return new_obj;
	}

	template <JsonValueType T> void insert(const std::string& key, T&& val)
	{
		insertRawPtr(key, new Value(val));
	}

	void insertRawPtr(const std::string& key, Element* val)
	{
		obj_[key] = val;
	}

	Element*& at(const std::string& key)
	{
		return obj_.at(key);
	}

	Element*& operator[](const std::string& key)
	{
		return obj_[key];
	}

	size_t size() const noexcept
	{
		return obj_.size();
	}

	object_t::const_iterator begin() const
	{
		return obj_.begin();
	}

	object_t::const_iterator end() const
	{
		return obj_.end();
	}

  private:
	void clear() override
	{
		for (const auto& [key, val] : obj_)
		{
			if (val)
			{
				delete val;
			}
		}
		obj_.clear();
	}
};
} // namespace json
} // namespace lynx

#endif