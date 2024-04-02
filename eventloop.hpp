#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include "nocopyable.h"
#include "timestamp.hpp"

class Channel;
class Poller;

using functor = std::function<void()>;
using ChannelList = std::vector<Channel*>;

class EventLoop : nocopyable
{
public:
    EventLoop(){}
private:
    std::atomic_bool isLooping_;
    std::atomic_bool quit_;
    std::atomic_bool callable_; //标识当前loop是否有需要执行的回调
    const std::thread::id threadID;
    TimeStamp PollreturnTime_;
    std::unique_ptr<Poller> poller_;
};