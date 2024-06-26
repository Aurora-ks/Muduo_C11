#pragma once

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>
#include "base/nocopyable.h"
#include "base/Thread.h"

class EventLoop;

class EventLoopThread : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& callback = ThreadInitCallback(), const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* StartLoop();
private:
    void ThreadFunciton();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};