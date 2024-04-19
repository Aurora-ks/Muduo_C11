#pragma once

#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include "net/Channel.h"
#include "net/Timer.h"
#include <vector>
#include <set>
#include <memory>
#include <tuple>

class EventLoop;

using TimerID = uint64_t;

struct ExpiredTimer
{
    Timestamp timestamp_;
    std::shared_ptr<Timer> timer_;

    ExpiredTimer(Timestamp timestamp, Timer *timer)
        : timestamp_(timestamp),
          timer_(timer) {}

    ExpiredTimer(Timestamp timestamp, const std::shared_ptr<Timer> &timer)
        : timestamp_(timestamp),
          timer_(timer) {}

    bool operator<(const ExpiredTimer &t) const
    {
        return timestamp_ < t.timestamp_
                   ? timestamp_ < t.timestamp_
                   : timer_->id() < t.timer_->id();
    }
};

class TimerQueue : nocopyable
{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerID AddTimer(TimerCallback f, Timestamp CountDown, int interval);
    void cancel(TimerID timerid);

private:
    using TimerList = std::set<ExpiredTimer>;
    using TimerPtr = std::shared_ptr<Timer>;

    void AddTimerInLoop(Timer *timer);
    void CancelTimerInLoop(TimerID timerid);

    void HandleRead();

    std::vector<ExpiredTimer> GetExpiredTimer(Timestamp now);
    void reset(const std::vector<ExpiredTimer> &expired, Timestamp now);
    bool insert(Timer *timer);
    bool insert(const TimerPtr &timer);

    EventLoop *loop_;
    const int timerfd_;
    Channel TimerChannel_;

    TimerList timers_;

    bool CallingExpiredTimers_;
    std::set<TimerID> CancelTimers_;
};