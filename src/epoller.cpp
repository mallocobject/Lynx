#include "lynx/include/epoller.h"
#include "lynx/include/channel.h"
#include "lynx/include/logger.h"
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
Epoller::Epoller() : epfd_(epoll_create1(0))
{
}

Epoller::~Epoller()
{
	close(epfd_);
}

void Epoller::updateChannel(Channel* ch)
{

	epoll_event ev{ch->getEvents(), {.ptr = ch}};
	if (ch->inEpoll())
	{
		epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->fd(), &ev);
	}
	else
	{
		epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd(), &ev);
		ch->setInEpoll(true);
	}
	events.push_back(epoll_event());
}

void Epoller::deleteChannel(Channel* ch)
{
	assert(ch->inEpoll());
	epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
	ch->setInEpoll(false);
	events.pop_back();
}

std::vector<Channel*> Epoller::wait(int timeout)
{
	while (true)
	{
		int nevs = epoll_wait(epfd_, events.data(), events.size(), timeout);
		if (nevs == -1)
		{
			if (errno == EINTR)
			{
				LOG_WARN() << "epoll_wait interrupted, continue...";
				continue;
			}
			else
			{
				LOG_FATAL() << "epoll_wait error [" << errno
							<< "]: " << strerror(errno);
				exit(1);
			}
		}
		else if (nevs == 0)
		{
			LOG_WARN() << "epoll_wait out time";
			// todo
		}
		else if (nevs > 0)
		{
			std::vector<Channel*> active_chs(nevs);
			for (int i = 0; i < nevs; i++)
			{
				Channel* ch = reinterpret_cast<Channel*>(events[i].data.ptr);
				ch->setRevents(events[i].events);
				active_chs[i] = ch;
			}
			return active_chs;
		}
	}
}
} // namespace lynx