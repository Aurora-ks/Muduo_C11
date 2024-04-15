#include "net/Poller.h"
#include "base/Logger.h"
#include <format>

Poller::Poller(EventLoop *loop)
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(InitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL << "epoll_creat error, errno:" << errno;
    }
}

Timestamp Poller::poll(int timeoutMs, ChannelList *ActivateChannels)
{
    int num = ::epoll_wait(epollfd_, &*events_.begin(), (int)events_.size(), timeoutMs);
    int errnum = errno;
    if (num > 0)
    {
        LOG_DEBUG << num << " events happened";
        FillActiveChannels(num, ActivateChannels);
        if (num == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (num == 0)
    {
        LOG_DEBUG << "nothing happened";
    }
    return Timestamp::now();
}

void Poller::UpdateChannel(Channel *channel)
{
    const int state = channel->state();
    LOG_DEBUG << std::format("update fd = {} events = {} state = {}", channel->fd(), channel->EventsToString(), state);
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

void Poller::RemoveChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    LOG_DEBUG << "remove fd = " << fd;
    int state = channel->state();
    if (state == ADD)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->SetState(NEW);
}

void Poller::FillActiveChannels(int num, ChannelList *ActiveChannels) const
{
    for (int i = 0; i < num; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->SetRevents(events_[i].events);
        ActiveChannels->emplace_back(channel);
    }
}

void Poller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::bzero(&event, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;
    LOG_DEBUG << std::format("epoll_ctl op = {} fd = {} event = {{{}}}", OperationToString(operation), fd, channel->EventsToString());
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR << std::format("epoll_ctl op = {} fd = {}", OperationToString(operation), fd);
        }
        else
        {
            LOG_FATAL << std::format("epoll_ctl op = {} fd = {}", OperationToString(operation), fd);
        }
    }
}

const char* Poller::OperationToString(int op) const
{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      return "Unknown Operation";
  }
}