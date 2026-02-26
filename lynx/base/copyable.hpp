#ifndef LYNX_BASE_COPYABLE_HPP
#define LYNX_BASE_COPYABLE_HPP

namespace lynx
{
namespace base
{
class copyable
{
  protected:
	copyable() = default;
	~copyable() = default;

	copyable(const copyable&) = default;
	copyable& operator=(const copyable&) = default;
};
} // namespace base
} // namespace lynx

#endif