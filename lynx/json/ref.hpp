#ifndef LYNX_REF_HPP
#define LYNX_REF_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/json/element.hpp"
namespace lynx
{
class Ref : public noncopyable
{
  private:
	Element* ptr_;

  public:
	Ref(Element* ptr = nullptr) : ptr_(ptr)
	{
	}
};
} // namespace lynx

#endif