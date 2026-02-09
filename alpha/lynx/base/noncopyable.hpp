#ifndef LYNX_NONCOPYABLE_HPP
#define LYNX_NONCOPYABLE_HPP

namespace lynx
{
class noncopyable
{
  protected:
	noncopyable() = default;
	~noncopyable() = default;

	noncopyable(const noncopyable&) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
};
} // namespace lynx

#endif