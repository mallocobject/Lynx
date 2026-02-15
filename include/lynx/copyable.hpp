#ifndef LYNX_PUBLIC_COPYABLE_HPP
#define LYNX_PUBLIC_COPYABLE_HPP

namespace lynx
{
class copyable
{
  protected:
	copyable() = default;
	~copyable() = default;

	copyable(const copyable&) = default;
	copyable& operator=(const copyable&) = default;
};
} // namespace lynx

#endif