#include "lynx/net/epoller.h"
#include "lynx/base/logger.hpp"
#include "lynx/base/time_stamp.h"
#include "lynx/net/buffer.h"
#include "lynx/net/channel.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

namespace lynx
{
static const int Init_Event_List_Size = 16;

Epoller::Epoller() : epfd_(epoll_create1(0)), events(Init_Event_List_Size)
{
}

Epoller::~Epoller()
{
	close(epfd_);
}

void Epoller::updateChannel(Channel* ch)
{

	epoll_event ev{ch->events(), {.ptr = ch}};
	if (ch->inEpoll())
	{
		epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->fd(), &ev);
	}
	else
	{
		epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd(), &ev);
		ch->setInEpoll(true);
	}
}

void Epoller::deleteChannel(Channel* ch)
{
	assert(ch->inEpoll());
	epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
	ch->setInEpoll(false);
}

TimeStamp Epoller::wait(std::vector<Channel*>* active_chs, int timeout)
{
	int nevs = epoll_wait(epfd_, events.data(), events.size(), timeout);
	TimeStamp time_stamp = TimeStamp::now();

	int saved_errno = errno;

	if (nevs == -1)
	{
		if (saved_errno != EINTR)
		{
			LOG_ERROR << "Epoller::wait [" << errno << "]: " << strerror(errno);
		}
	}
	else if (nevs == 0)
	{
		LOG_WARN << "epoll_wait out time";
		// todo
	}
	else
	{
		// LOG_DEBUG << nevs << " events happended";
		for (int i = 0; i < nevs; i++)
		{
			Channel* ch = reinterpret_cast<Channel*>(events[i].data.ptr);
			// LOG_DEBUG << events[i].events;
			ch->setRevents(events[i].events);
			active_chs->push_back(ch);
		}

		if (nevs == events.size())
		{
			events.resize(events.size() * 2);
		}
	}

	return time_stamp;
}
} // namespace lynx