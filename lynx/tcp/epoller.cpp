#include "lynx/tcp/epoller.h"
#include "lynx/logger/logger.h"
#include "lynx/tcp/channel.h"
#include "lynx/time/time_stamp.h"
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

using namespace lynx;

static const int kInitEventSize = 16;

Epoller::Epoller() : evs_(kInitEventSize)
{
	epfd_ = ::epoll_create1(0);
	if (epfd_ == -1)
	{
		LOG_FATAL << "epoll_create1 failed: " << ::strerror(errno);
	}
}

Epoller::~Epoller()
{
	if (epfd_ != -1)
	{
		::close(epfd_);
		epfd_ = -1;
	}
}

void Epoller::updataChannel(Channel* ch)
{
	epoll_event ev{.events = ch->events(), .data{.ptr = ch}};
	if (ch->inEpoll())
	{
		LOG_TRACE << ch->events();
		::epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->fd(), &ev);
	}
	else
	{
		LOG_TRACE << ch->events();
		::epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd(), &ev);
		ch->setInEpoll(true);
	}
}

void Epoller::removeChannel(Channel* ch)
{
	assert(ch->inEpoll());
	::epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
	ch->setInEpoll(false);
}

TimeStamp Epoller::poll(std::vector<Channel*>* active_chs, int timeout)
{
	int nevs = ::epoll_wait(epfd_, evs_.data(), evs_.size(), timeout);
	TimeStamp now = TimeStamp::now();

	int saved_errno = errno;

	if (nevs == -1)
	{
		if (saved_errno != EINTR)
		{
			LOG_ERROR << "epoll_wait failed: " << ::strerror(errno);
		}
	}
	else if (nevs == 0)
	{
		LOG_WARN << "epoll_wait out time";
	}
	else
	{
		for (int i = 0; i < nevs; i++)
		{
			Channel* ch = reinterpret_cast<Channel*>(evs_[i].data.ptr);
			ch->setRevents(evs_[i].events);
			active_chs->push_back(ch);
		}

		if (nevs == evs_.size())
		{
			evs_.resize(evs_.size() * 2);
		}
	}

	return now;
}
