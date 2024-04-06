#include "EventLoop.h"
#include "Logger.h"
#include "EpollPoller.h"

thread_local EventLoop* LoopInThread = nullptr;

EventLoop::EventLoop()
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
    if (LoopInThread)
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

EventLoop::~EventLoop()
{
    WakeupChannel_->DisableAll();
    WakeupChannel_->remove();
    ::close(wakeupFd_);
    LoopInThread = nullptr;
}

void EventLoop::start()
{
    isLooping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start\n", this)
    while (!quit_)
    {
        ActiveChannels_.clear();
        PollreturnTime_ = poller_->poll(PollTimeMs, &ActiveChannels_);
        for (auto channel : ActiveChannels_)
        {
            channel->HandleEvent(PollreturnTime_);
        }
        DoCallback();
    }
    LOG_INFO("EventLoop %p in thread %d stoped\n", this, std::this_thread::get_id())
    isLooping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // 如果在其他线程中调用了quit
    if (!IsInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::RunInLoop(functor callback)
{
    if (IsInLoopThread())
    {
        callback();
    }
    else
    {
        QueueInLoop(callback);
    }
}

void EventLoop::QueueInLoop(functor callback)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors_.emplace_back(callback);
    }
    if (!IsInLoopThread() || callable_)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", n)
    }
}

void EventLoop::UpdateChannel(Channel *channel)
{
    poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel)
{
    poller_->RemoveChannel(channel);
}
bool EventLoop::HasChannel(Channel *channel)
{
    poller_->hasChannel(channel);
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::HandleRead() reads %d bytes instead of 8", n)
    }
}
// 执行回调
void EventLoop::DoCallback()
{
    std::vector<functor> fun;
    callable_ = true;
    {
        // 将functors_清空，函数内执行回调，可继续向functors_注册回调，实现并行
        std::lock_guard<std::mutex> lock(mutex_);
        fun.swap(functors_);
    }
    for (const functor &callback : fun)
    {
        callback();
    }
    callable_ = false;
}