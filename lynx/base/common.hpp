#ifndef LYNX_COMMON_H
#define LYNX_COMMON_H

namespace unitary
{
#define DISABLE_DEFAULT(cname) cname() = delete;

#define DISABLE_COPY(cname)                                                    \
	cname(const cname&) = delete;                                              \
	cname& operator=(const cname&) = delete;

#define DISABLE_MOVE(cname)                                                    \
	cname(cname&&) = delete;                                                   \
	cname& operator=(cname&&) = delete;

#define DISABLE_COPY_AND_MOVE(cname)                                           \
	DISABLE_COPY(cname)                                                        \
	DISABLE_MOVE(cname)
} // namespace unitary

#endif