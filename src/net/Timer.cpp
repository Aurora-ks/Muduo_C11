#include "net/Timer.h"

std::atomic_uint64_t Timer::TotalNum_(0);

Timer::Timer(TimerCallback f, Timestamp CountDown, int interval)
    : callback_(std::move(f)),
      expiration_(CountDown),
      interval_(interval),
      repeat_(interval_ > 0),
      id_(++TotalNum_)
{
}

void Timer::restart(Timestamp now)
{
    if(repeat_) expiration_ = now + interval_;
    else expiration_ = Timestamp();
}