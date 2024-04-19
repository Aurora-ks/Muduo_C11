#pragma once

#include "base/nocopyable.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include <atomic>

class Timer : nocopyable
{
public:
    Timer(TimerCallback f, Timestamp CountDown, int interval);

    void run() const { callback_(); }
    void restart(Timestamp now);

    Timestamp expiration() const { return expiration_; }
    bool interval() const { return interval_; }
    bool repeat() const { return repeat_; }
    uint64_t id() const { return id_; }

    static uint64_t TotalNum() { return TotalNum_.load(); }

private:
    TimerCallback callback_;
    Timestamp expiration_;
    const int interval_; // ms
    const bool repeat_;
    const uint64_t id_;

    static std::atomic_uint64_t TotalNum_;
};