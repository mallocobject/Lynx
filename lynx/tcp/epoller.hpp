#ifndef LYNX_TCP_EPOLLER_HPP
#define LYNX_TCP_EPOLLER_HPP

#include "lynx/base/noncopyable.hpp"
#include "lynx/time/time_stamp.hpp"
#include <sys/epoll.h>
#include <vector>
namespace lynx
{
namespace tcp
{
class Channel;
class Epoller : public base::noncopyable
{
  private:
	int epfd_;
	std::vector<epoll_event> evs_;

  public:
	Epoller();
	~Epoller();

	void updataChannel(Channel* ch);
	void removeChannel(Channel* ch);

	time::TimeStamp poll(std::vector<Channel*>* active_chs, int timeout = -1);
};
} // namespace tcp
} // namespace lynx

#endif