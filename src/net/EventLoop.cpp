#include "net/EventLoop.h"
#include "net/Poller.h"
#include "net/TimerQueue.h"
#include "base/Logger.h"
#include "base/Thread.h"
#include <format>

thread_local EventLoop *LoopInThread = nullptr;

// 创建wakeupfd，用来唤醒sub处理新的channel
static int CreateEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL << std::format("create eventfd error:{}", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : isLooping_(false),
      quit_(false),
      callable_(false),
      threadID_(std::this_thread::get_id()),
      poller_(std::unique_ptr<Poller>(new Poller(this))),
      TimerQueue_(new TimerQueue(this)),
      wakeupFd_(CreateEventfd()),
      WakeupChannel_(std::unique_ptr<Channel>(new Channel(this, wakeupFd_))),
      CurrentActiveChannel_(nullptr)
{
    auto tid = std::this_thread::get_id();

    LOG_DEBUG << std::format("EventLoop created {} in thread {}",
                             reinterpret_cast<unsigned long>(this), Thread::CurrentThreadId());
    if (LoopInThread)
    {
        LOG_FATAL << std::format("Another EventLoop {} exists in this thread {}",
                                 reinterpret_cast<unsigned long>(LoopInThread), Thread::CurrentThreadId());
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
    LOG_DEBUG << std::format("EventLoop {} destructs in thread {}",
                             reinterpret_cast<unsigned long>(LoopInThread), Thread::CurrentThreadId());
    WakeupChannel_->DisableAll();
    WakeupChannel_->remove();
    ::close(wakeupFd_);
    LoopInThread = nullptr;
}

void EventLoop::start()
{
    isLooping_ = true;
    quit_ = false;
    LOG_DEBUG << std::format("EventLoop {} start", reinterpret_cast<unsigned long>(this));
    while (!quit_)
    {
        ActiveChannels_.clear();
        PollreturnTime_ = poller_->poll(PollTimeMs, &ActiveChannels_);

        for (auto channel : ActiveChannels_)
        {
            channel->HandleEvent(PollreturnTime_);
        }
        if(!functors_.empty()) DoCallback();
    }
    LOG_DEBUG << std::format("EventLoop {} stop", reinterpret_cast<unsigned long>(this));
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
        LOG_ERROR << std::format("EventLoop::wakeup() writes {} bytes instead of 8", n);
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
    return poller_->hasChannel(channel);
}

TimerID EventLoop::RunAt(Timestamp time, TimerCallback f)
{
    return TimerQueue_->AddTimer(std::move(f), time, 0);
}

TimerID EventLoop::RunAfterS(int delay, TimerCallback f)
{
    Timestamp time = Timestamp::now() + delay * 1000;
    return RunAt(time, std::move(f));
}

TimerID EventLoop::RunAfterMs(int delay, TimerCallback f)
{
    Timestamp time = Timestamp::now() + delay;
    return RunAt(time, std::move(f));
}

TimerID EventLoop::RunEveryS(int delay, TimerCallback f)
{
    Timestamp time = Timestamp::now() + delay * 1000;
    return TimerQueue_->AddTimer(std::move(f), time, delay*1000);
}

TimerID EventLoop::RunEveryMs(int delay, TimerCallback f)
{
    Timestamp time = Timestamp::now() + delay * 1000;
    return TimerQueue_->AddTimer(std::move(f), time, delay);
}

void EventLoop::CancelTimer(TimerID timerid)
{
    TimerQueue_->cancel(timerid);
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << std::format("EventLoop::handleRead() reads {} bytes instead of 8", n);
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