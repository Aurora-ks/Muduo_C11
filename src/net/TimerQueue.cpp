#include "net/TimerQueue.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "base/Logger.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <format>
#include <algorithm>
#include <strings.h>

using namespace std;

int CreateTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
        LOG_FATAL << "Failed in timerfd_create";
    return timerfd;
}

void ResetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec val;
    bzero(&val, sizeof(val));
    time_t sec = (expiration - Timestamp::now()) / 1000;
    long nsec = (expiration - Timestamp::now()) % 1000;
    val.it_value.tv_sec = sec;
    val.it_value.tv_nsec = nsec * 1000;
    if (::timerfd_settime(timerfd, 0, &val, NULL))
    {
        LOG_FATAL << "timerfd_settime()";
    }
}

void ReadFromTimerfd(int timerfd)
{
    uint64_t one;
    ssize_t n = ::read(timerfd, &one, sizeof(one));
    if (n != sizeof(one))
        LOG_ERROR << format("TimerQueue::HandleRead reads {} bytes instead of 8", n);
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(CreateTimerfd()),
      TimerChannel_(loop, timerfd_),
      timers_(),
      CallingExpiredTimers_(false),
      CancelTimers_()
{
    TimerChannel_.SetReadCallback(bind(&TimerQueue::HandleRead, this));
    TimerChannel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    TimerChannel_.DisableAll();
    TimerChannel_.remove();
    ::close(timerfd_);
}

TimerID TimerQueue::AddTimer(TimerCallback f, Timestamp CountDown, int interval)
{
    Timer *timer = new Timer(move(f), CountDown, interval);
    loop_->RunInLoop(bind(&TimerQueue::AddTimerInLoop, this, timer));
    return timer->id();
}

void TimerQueue::cancel(TimerID timerid)
{
    loop_->RunInLoop(bind(&TimerQueue::CancelTimerInLoop, this, timerid));
}

void TimerQueue::AddTimerInLoop(Timer *timer)
{
    if (insert(timer))
    {
        ResetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::CancelTimerInLoop(TimerID timerid)
{
    auto it = find_if(timers_.begin(), timers_.end(), [timerid](ExpiredTimer i)
            { return i.timer_->id() == timerid; });

    if(it != timers_.end())
    {
        timers_.erase(it);
    }
    else if(CallingExpiredTimers_)
    {
        CancelTimers_.insert(timerid);
    }
}

void TimerQueue::HandleRead()
{
    Timestamp now = Timestamp::now();
    ReadFromTimerfd(timerfd_);

    auto expired = GetExpiredTimer(now);

    CallingExpiredTimers_ = false;
    for (const auto &timer : expired)
    {
        timer.timer_->run();
    }
    CallingExpiredTimers_ = false;

    reset(expired, now);
}

vector<ExpiredTimer> TimerQueue::GetExpiredTimer(Timestamp now)
{
    vector<ExpiredTimer> expired;
    auto end = find_if(timers_.begin(), timers_.end(), [now](ExpiredTimer i)
                       { return i.timestamp_ > now; });
    copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    return expired;
}

void TimerQueue::reset(const vector<ExpiredTimer> &expired, Timestamp now)
{
    for(const auto &timer : expired)
    {
        if(timer.timer_->repeat() && CancelTimers_.find(timer.timer_->id()) == CancelTimers_.end())
        {
            timer.timer_->restart(now);
            insert(timer.timer_);
        }
    }

    if(!timers_.empty())
    {
        Timestamp next = timers_.begin()->timer_->expiration();
        ResetTimerfd(timerfd_, next);
    }
}

bool TimerQueue::insert(Timer *timer)
{
    bool EarliestChanged = false;
    Timestamp expired = timer->expiration();
    if (timers_.empty() || expired < timers_.begin()->timestamp_)
        EarliestChanged = true;

    timers_.insert({expired, timer});

    return EarliestChanged;
}

bool TimerQueue::insert(const TimerQueue::TimerPtr &timer)
{
    bool EarliestChanged = false;
    Timestamp expired = timer->expiration();
    if (timers_.empty() || expired < timers_.begin()->timestamp_)
        EarliestChanged = true;

    timers_.insert({expired, timer});

    return EarliestChanged;
}