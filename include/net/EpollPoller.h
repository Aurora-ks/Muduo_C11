#pragma once

#include <vector>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <error.h>
#include "Poller.h"
#include "../base/Timestamp.h"

class EventLoop;
class Channel;

using EventList = std::vector<epoll_event>;

enum states
{
    NEW = 0,
    ADD = 1,
    DELETED = 2
};

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override { ::close(epollfd_); }

    Timestamp poll(int timeoutMs, ChannelList *ActivateChannels) override;
    void UpdateChannel(Channel *channel) override;
    void RemoveChannel(Channel *channel) override;

private:
    void FillActiveChannels(int num, ChannelList *ActiveChannels) const;
    void update(int operation, Channel *channel);

    static const int InitEventListSize = 16;
    int epollfd_;
    EventList events_;
};