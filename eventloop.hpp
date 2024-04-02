#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <errno.h>
#include "nocopyable.h"
#include "timestamp.hpp"
#include "logger.hpp"
#include "epollpoller.hpp"

class Channel;
class Poller;

using functor = std::function<void()>;
using ChannelList = std::vector<Channel* >;

const int PollTimeMs = 10000;
thread_local EventLoop* LoopInThread = nullptr;

class EventLoop : nocopyable
{
public:
    EventLoop()
        : isLooping_(false),
          quit_(false),
          callable_(false),
          threadID_(std::this_thread::get_id()),
          poller_(new EPollPoller(this)),
          WakeupChannel_(new Channel(this, wakeupFd_)),
          CurrentActiveChannel_(nullptr)
    {
        // 创建wakeupfd，用来唤醒sub处理新的channel
        wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (wakeupFd_ < 0)
        {
            LOG_FATAL("eventfd creat error:%d\n", errno)
        }
        LOG_DEBUG("eventloop %p created in thread %d\n", this, std::this_thread::get_id())
        if(LoopInThread)
        {
            LOG_FATAL("other loop %p already exists in thread %d", LoopInThread, std::this_thread::get_id())
        }
        else
        {
            LoopInThread = this;
        }
        WakeupChannel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
        WakeupChannel_->EnableReading();
    }

    ~EventLoop()
    {
        WakeupChannel_->DisableAll();
        WakeupChannel_->remove();
        ::close(wakeupFd_);
        LoopInThread = nullptr;
    }
    // 开启事件循环
    void start()
    {
        isLooping_ = true;
        quit_ = false;
        LOG_INFO("EventLoop %p start\n", this)
        while(!quit_)
        {
            ActiveChannels_.clear();
            PollreturnTime_ = poller_->poll(PollTimeMs, &ActiveChannels_);
            for(auto channel : ActiveChannels_)
            {
                channel->HandleEvent(PollreturnTime_);
            }
            DoCallback();
        }
        LOG_INFO("EventLoop %p in thread %d stoped\n", this, std::this_thread::get_id())
        isLooping_ = false;
    }

    // 退出事件循环
    void quit()
    {
        quit_ = true;
        //如果在其他线程中调用了quit
        if(!IsInLoopThread())
        {
            wakeup();
        }
    }

    TimeStamp PollreturnTime() { return PollreturnTime_; }
    // 在当前loop中执行
    void RunInLoop(functor callback)
    {
        if(IsInLoopThread())
        {
            callback();
        }
        else
        {
            QueueInLoop(callback);
        }
    }
    // 放入队列，唤醒当前loop线程执行
    void QueueInLoop(functor callback)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            functors_.emplace_back(callback);
        }
        if(!IsInLoopThread() || callable_)
        {
            wakeup();
        }
    }
    // 唤醒loop所在线程
    void wakeup()
    {
        uint64_t one = 1;
        ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
        if(n != sizeof(one))
        {
            LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", n)
        }
    }
    // channel通过loop调用poller
    void UpdateChannel(Channel *channel)
    {
        poller_->UpdateChannel(channel);
    }
    void RemoveChannel(Channel *channel)
    {
        poller_->RemoveChannel(channel);
    }
    bool HasChannel(Channel *channel)
    {

    }

    bool IsInLoopThread() const { return threadID_ == std::this_thread::get_id(); }

private:
    // 唤醒
    void HandleRead()
    {
        uint64_t one = 1;
        ssize_t n = read(wakeupFd_, &one, sizeof(one));
        if(n != sizeof(one))
        {
            LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", n)
        }
    }
    // 执行回调
    void DoCallback()
    {
        std::vector<functor> fun;
        callable_ = true;
        {
            //将functors_清空，函数内执行回调，可继续向functors_注册回调，实现并行
            std::lock_guard<std::mutex> lock(mutex_);
            fun.swap(functors_);
        }
        for(const functor& callback : fun)
        {
            callback();
        }
        callable_ = false;
    }
    std::atomic_bool isLooping_;
    std::atomic_bool quit_;
    std::atomic_bool callable_; // 标识当前loop是否有需要执行的回调
    const std::thread::id threadID_;
    TimeStamp PollreturnTime_;
    std::unique_ptr<EPollPoller> poller_;
    int wakeupFd_; // 选择轮询选择一个subloop，通过此成员唤醒
    std::unique_ptr<Channel> WakeupChannel_;
    ChannelList ActiveChannels_;
    Channel *CurrentActiveChannel_;
    std::vector<functor> functors_; // 存储loop需要执行的所有回调
    std::mutex mutex_;
};