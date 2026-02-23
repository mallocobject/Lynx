#ifndef LYNX_OBJECT_HPP
#define LYNX_OBJECT_HPP

#include "lynx/json/element.hpp"
#include "lynx/json/value.hpp"
#include <map>
#include <string>
namespace lynx
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

	bool isObject() const noexcept override
	{
		return true;
	}

	Object* asObject() override
	{
		return this;
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

	Element*& at(const std::string& key)
	{
		return obj_.at(key);
	}

	Element*& operator[](const std::string& key)
	{
		return obj_[key];
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

	void insertRawPtr(const std::string& key, Element* val)
	{
		obj_[key] = val;
	}
};
} // namespace lynx

#endif