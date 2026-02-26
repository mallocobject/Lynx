#ifndef LYNX_JSON_REF_HPP
#define LYNX_JSON_REF_HPP

#include "lynx/base/copyable.hpp"
#include "lynx/json/array.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/object.hpp"
#include "lynx/json/value.hpp"
#include "lynx/logger/logger.hpp"
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
namespace lynx
{
namespace json
{
class Ref : public base::copyable
{
  private:
	std::shared_ptr<Element> ptr_;

  public:
	// Ref(Element* ptr = nullptr) : ptr_(ptr)
	// {
	// }

	explicit Ref(std::shared_ptr<Element> ptr) : ptr_(std::move(ptr))
	{
	}

	Ref(Ref&& rhs) : ptr_(std::move(rhs.ptr_))
	{
		rhs.ptr_ = nullptr;
	}

	Ref& operator=(Ref&& rhs) noexcept
	{
		if (this == &rhs)
		{
			return *this;
		}

		ptr_ = std::move(rhs.ptr_);
		rhs.ptr_ = nullptr;

		return *this;
	}

	Ref operator[](const std::string& key)
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}
		if (auto obj = std::dynamic_pointer_cast<Object>(ptr_))
		{
			return Ref(obj->at(key));
		}
		LOG_FATAL << "Not an object";
		throw std::runtime_error("Not an object");
	}

	Ref operator[](size_t index)
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}
		if (auto arr = std::dynamic_pointer_cast<Array>(ptr_))
		{
			return Ref(arr->at(index));
		}
		LOG_FATAL << "Not an array";
		throw std::runtime_error("Not an array");
	}

	bool asBool() const
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}

		return ptr_->asValue()->asBool();
	}

	int64_t asInt() const
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}

		return ptr_->asValue()->asInt();
	}

	double asFloat() const
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}

		return ptr_->asValue()->asFloat();
	}

	std::string asStr() const
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}

		return ptr_->asValue()->asStr();
	}

	std::nullptr_t asNull() const
	{
		if (!ptr_)
		{
			LOG_FATAL << "Null reference";
			throw std::runtime_error("Null reference");
		}

		return ptr_->asValue()->asNull();
	}

	Element* get() const
	{
		return ptr_.get();
	}

	std::shared_ptr<Element> getShared() const
	{
		return ptr_;
	}

	std::string serialize() const
	{
		return ptr_->serialize();
	}
};

template <JsonValueType T> Ref make_value(T&& val)
{
	return Ref(std::make_shared<Value>(std::forward<T>(val)));
}

inline Ref make_array(std::initializer_list<Ref> ref_list)
{
	auto arr = std::make_shared<Array>();
	for (auto& v : ref_list)
	{
		arr->append(v.getShared());
	}
	return Ref(arr);
}

struct Pair
{
	std::string key;
	Ref value;

	Pair(const char* k, Ref v) : key(k), value(std::move(v))
	{
	}

	Pair(std::string k, Ref v) : key(std::move(k)), value(std::move(v))
	{
	}
};

inline Ref make_object(std::initializer_list<Pair> ref_dict)
{
	auto obj = std::make_shared<Object>();
	for (auto& kv : ref_dict)
	{
		obj->insert(kv.key, kv.value.getShared());
	}
	return Ref(obj);
}

inline std::ostream& operator<<(std::ostream& os, const Ref& ref)
{
	Element* e = ref.get();
	if (e->isArray())
	{
		auto arr = e->asArray();
		os << '[';
		bool first = true;

		for (const auto& v : *arr)
		{
			if (!first)
			{
				os << ',';
			}
			os << Ref(v); // 递归调用 operator<<
			first = false;
		}
		os << ']';
		return os;
	}
	else if (e->isObject())
	{
		auto obj = e->asObject();
		os << '{';
		bool first = true;

		for (const auto& kv : *obj)
		{
			if (!first)
			{
				os << ',';
			}
			os << '\"' << kv.first << "\":" << Ref(kv.second);
			first = false;
		}
		os << '}';
		return os;
	}

	auto val = e->asValue();
	if (val->isBool())
	{
		os << (val->asBool() ? "true" : "false");
	}
	else if (val->isInt())
	{
		os << val->asInt();
	}
	else if (val->isFloat())
	{
		os << val->asFloat();
	}
	else if (val->isStr())
	{
		os << '\"' << val->asStr() << '\"';
	}
	else if (val->isNull())
	{
		os << "null";
	}
	else
	{
		LOG_WARN << "Unsupported type to print";
		os << "null";
	}

	return os;
}
} // namespace json
} // namespace lynx

template <>
struct std::formatter<lynx::json::Ref> : std::formatter<std::string_view>
{
	auto format(const lynx::json::Ref& ref, std::format_context& ctx) const
	{
		std::ostringstream oss;
		oss << ref;
		return std::formatter<std::string_view>::format(oss.str(), ctx);
	}
};

#endif