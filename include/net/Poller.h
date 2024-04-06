#pragma once

#include <vector>
#include <unordered_map>
#include "Channel.h"

class EventLoop;
class TimeStamp;

class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop): loop_(loop){}
    virtual ~Poller() = default;

    virtual TimeStamp poll(int timeoutMs, ChannelList* ActivateChannels) = 0;
    virtual void UpdateChannel(Channel* channel) = 0;
    virtual void RemoveChannel(Channel* channel) = 0;

    bool hasChannel(Channel* channel)
    {
        auto it = channels_.find(channel->fd());
        return it != channels_.end() && it->second == channel;
    }

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;

    ChannelMap channels_;

private:
    EventLoop* loop_;
};