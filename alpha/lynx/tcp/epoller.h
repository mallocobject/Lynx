#ifndef LYNX_EPOLLER_H
#define LYNX_EPOLLER_H

#include "lynx/base/noncopyable.hpp"
#include "lynx/base/time_stamp.h"
#include <sys/epoll.h>
#include <vector>
namespace lynx
{
class Channel;
class Epoller : public noncopyable
{
  private:
	int epfd_;
	std::vector<epoll_event> evs_;

  public:
	Epoller();
	~Epoller();

	void updataChannel(Channel* ch);
	void removeChannel(Channel* ch);

	TimeStamp poll(std::vector<Channel*>* active_chs, int timeout = -1);
};
} // namespace lynx

#endif