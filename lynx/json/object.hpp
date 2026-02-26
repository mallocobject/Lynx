#ifndef LYNX_JSON_OBJECT_HPP
#define LYNX_JSON_OBJECT_HPP

#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <cstddef>
#include <format>
#include <map>
#include <memory>
#include <string>
#include <utility>
namespace lynx
{
namespace json
{
using object_t = std::map<std::string, std::shared_ptr<Element>>;

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
	}

	// static void* operator new(size_t size)
	// {
	// 	return base::alloc::allocate(size);
	// }

	// static void operator delete(void* p, size_t size)
	// {
	// 	base::alloc::deallocate(p, size);
	// }

	bool isObject() const noexcept override
	{
		return true;
	}

	std::shared_ptr<Object> asObject() override
	{
		return std::dynamic_pointer_cast<Object>(shared_from_this());
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

	std::shared_ptr<Element> copy() const override
	{
		auto new_obj = std::make_shared<Object>();
		for (const auto& [key, val] : obj_)
		{
			new_obj->insert(key, val->copy());
		}

		return new_obj;
	}

	template <JsonValueType T> void insert(const std::string& key, T&& val)
	{
		insert(key, std::make_shared<Element>(std::forward<T>(val)));
	}

	void insert(const std::string& key, std::shared_ptr<Element> val)
	{
		obj_[key] = std::move(val);
	}

	std::shared_ptr<Element>& at(const std::string& key)
	{
		return obj_.at(key);
	}

	const std::shared_ptr<Element>& operator[](const std::string& key) const
	{
		return obj_.at(key);
	}

	std::shared_ptr<Element>& operator[](const std::string& key)
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
};
} // namespace json
} // namespace lynx

#endif