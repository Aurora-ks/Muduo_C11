#pragma once

#include "base/nocopyable.h"
#include "net/Callbacks.h"
#include "net/Channel.h"
#include <set>

class EventLoop;

class Signals : nocopyable
{
public:
    Signals(EventLoop *loop, std::initializer_list<int> SignalList);
    ~Signals();

    void wait(const SignalCallback &f);
private:
    void HandleRead();

    EventLoop *loop_;
    int SignalFd_;
    Channel SignalChannel_;
    SignalCallback callback_;
};