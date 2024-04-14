#pragma once

#include <vector>
#include <unordered_map>
#include <vector>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <error.h>
#include "Channel.h"

class EventLoop;
class Timestamp;

class Poller
{
public:
    enum states
    {
        NEW = 0,
        ADD = 1,
        DELETED = 2
    };
    using ChannelList = std::vector<Channel*>;
    using EventList = std::vector<epoll_event>;

    Poller(EventLoop* loop);
    ~Poller() { ::close(epollfd_); }

    Timestamp poll(int timeoutMs, ChannelList* ActivateChannels);
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);

    bool hasChannel(Channel* channel)
    {
        auto it = channels_.find(channel->fd());
        return it != channels_.end() && it->second == channel;
    }

private:
    void FillActiveChannels(int num, ChannelList *ActiveChannels) const;
    void update(int operation, Channel *channel);

    //for debug
    const char* OperationToString(int op) const;

    static const int InitEventListSize = 16;

    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

    int epollfd_;
    EventList events_;
    EventLoop* loop_;
};