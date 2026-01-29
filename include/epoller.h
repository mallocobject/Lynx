#ifndef LYNX_EPOLLER_H
#define LYNX_EPOLLER_H

#include "lynx/include/channel.h"
#include "lynx/include/common.h"
#include <sys/epoll.h>
#include <vector>

namespace lynx
{
class Channel;
class Epoller
{
  private:
	int epfd_; // epoll file descripter
	std::vector<epoll_event> events;

  public:
	DISABLE_COPY(Epoller)

	Epoller();
	~Epoller();
	void updateChannel(Channel* ch);
	void deleteChannel(Channel* ch);
	void wait(std::vector<Channel*>* active_chs, int timeout = -1);
};
} // namespace lynx

#endif