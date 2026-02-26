#ifndef LYNX_BASE_NONCOPYABLE_HPP
#define LYNX_BASE_NONCOPYABLE_HPP

namespace lynx
{
namespace base
{
class noncopyable
{
  protected:
	noncopyable() = default;
	~noncopyable() = default;

	noncopyable(const noncopyable&) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
};
} // namespace base
} // namespace lynx

#endif