#include "epollpoller.h"
#include "logger.h"

EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop),
                                            epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                                            events_(InitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_creat error:%d\n", errno)
    }
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *ActivateChannels)
{
    int num = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
    int errnum = errno;
    if (num > 0)
    {
        FillActiveChannels(num, ActivateChannels);
        if (num == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (num == 0)
    {
        LOG_DEBUG("%s timeout\n", __FUNCTION__)
    }
    return TimeStamp::now();
}

void EPollPoller::UpdateChannel(Channel *channel)
{
    const int state = channel->state();
    if (state == NEW || state == DELETED)
    {
        if (state == NEW)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->SetState(ADD);
        update(EPOLL_CTL_ADD, channel);
    }
    else // state == ADD
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->SetState(DELETED);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::RemoveChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    int state = channel->state();
    if (state == ADD)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->SetState(NEW);
}

void EPollPoller::FillActiveChannels(int num, ChannelList *ActiveChannels)
{
    for (int i = 0; i < num; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->SetRevents(events_[i].events);
        ActiveChannels->emplace_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::bzero(&event, sizeof(event));
    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    event.data.fd = fd;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl_del error:%d\n", errno)
        }
        else
        {
            LOG_FATAL("epoll_ctl_add/mod error:%d\n", errno)
        }
    }
}