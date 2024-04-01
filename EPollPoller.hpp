#pragma once

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <error.h>
#include "poller.hpp"
#include "timestamp.hpp"
#include "logger.hpp"

class EventLoop;
class Channel;

using EventList = std::vector<epoll_event>;

enum states
{
    NEW = 0,
    ADD = 1,
    DEL = 2
};

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop) : Poller(loop),
                                   epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                                   events_(InitEventListSize)
    {
        if (epollfd_ < 0)
        {
            LOG_FATAL("epoll_creat error:%d\n", errno)
        }
    }

    ~EPollPoller() override { ::close(epollfd_); }

    TimeStamp poll(int timeoutMs, ChannelList *ActivateChannels) override
    {
    }
    void UpdateChannel(Channel *channel) override
    {
    }
    void RemoveChannel(Channel *channel) override
    {
    }

private:
    void GetActiveChannels(int num, ChannelList *ActiveChannels)
    {
    }
    void update(int operation, Channel *channel)
    {
    }

    static const int InitEventListSize = 16;
    int epollfd_;
    EventList events_;
};