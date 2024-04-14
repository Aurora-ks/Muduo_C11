#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <errno.h>
#include "../base/nocopyable.h"
#include "../base/Timestamp.h"

class Channel;
class Poller;
class Poller;

using functor = std::function<void()>;
using ChannelList = std::vector<Channel* >;

const int PollTimeMs = 10000;

class EventLoop : nocopyable
{
public:
    EventLoop();

    ~EventLoop();
    // 开启事件循环
    void start();

    // 退出事件循环
    void quit();

    Timestamp PollreturnTime() { return PollreturnTime_; }
    // 在当前loop中执行
    void RunInLoop(functor callback);
    // 放入队列，唤醒当前loop线程执行
    void QueueInLoop(functor callback);
    // 唤醒loop所在线程
    void wakeup();
    // channel通过loop调用poller
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);

    bool IsInLoopThread() const { return threadID_ == std::this_thread::get_id(); }

private:
    // 唤醒
    void HandleRead();
    // 执行回调
    void DoCallback();

    std::atomic_bool isLooping_;
    std::atomic_bool quit_;
    std::atomic_bool callable_; // 标识当前loop是否有需要执行的回调
    const std::thread::id threadID_;
    Timestamp PollreturnTime_;
    std::unique_ptr<Poller> poller_;
    int wakeupFd_; // 选择轮询选择一个subloop，通过此成员唤醒
    std::unique_ptr<Channel> WakeupChannel_;
    ChannelList ActiveChannels_;
    Channel *CurrentActiveChannel_;
    std::vector<functor> functors_; // 存储loop需要执行的所有回调
    std::mutex mutex_;
};