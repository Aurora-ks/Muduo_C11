#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "nocopyable.h"

class EventLoop;
class EventLoopThread;


class EventLoopThreadPool : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *BaseLoop, const std::string &name);
    ~EventLoopThreadPool() = default;

    void SetThreadNum(int num) { ThreadNum_ = num;}

    void start(const ThreadInitCallback &callback = ThreadInitCallback());

    EventLoop* GetNextLoop();

    std::vector<EventLoop*> AllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }

private:
    EventLoop *BaseLoop_;
    std::string name_;
    bool started_;
    int ThreadNum_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};