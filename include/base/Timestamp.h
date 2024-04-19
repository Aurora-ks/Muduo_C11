#pragma once

#include <string>
#include <stdint.h>

class Timestamp
{
public:
    Timestamp(): microSecondsSinceEpoch_(0) {}
    explicit Timestamp(int64_t time): microSecondsSinceEpoch_(time) {}
    std::string toString() const;
    void swap(Timestamp &other)
    {
        std::swap(microSecondsSinceEpoch_, other.microSecondsSinceEpoch_);
    }

    int64_t GetTime() const { return microSecondsSinceEpoch_; }

    static Timestamp now();

    Timestamp operator + (const Timestamp& other) const
    {
        return Timestamp(microSecondsSinceEpoch_+other.GetTime());
    }

    Timestamp operator + (int time) const
    {
        return Timestamp(microSecondsSinceEpoch_+time);
    }

    int64_t operator - (const Timestamp &other) const
    {
        return microSecondsSinceEpoch_- other.GetTime();
    }

    bool operator < (const Timestamp &other) const
    {
        return microSecondsSinceEpoch_ < other.GetTime();
    }

    bool operator > (const Timestamp &other) const
    {
        return microSecondsSinceEpoch_ > other.GetTime();
    }

private:
    int64_t microSecondsSinceEpoch_;
};