#pragma once

#include <string>
#include <time.h>
#include <stdint.h>

class TimeStamp
{
public:
    TimeStamp(): microSecondsSinceEpoch_(0) {}
    explicit TimeStamp(int64_t time): microSecondsSinceEpoch_(time) {}
    std::string toString() const
    {
        char buf[64]{0};
        auto time = localtime(&microSecondsSinceEpoch_);
        snprintf(buf, 64, "%4d/%02d/%02d %02d:%02d:%02d",
                time->tm_year + 1900,
                time->tm_mon + 1,
                time->tm_mday,
                time->tm_hour,
                time->tm_min,
                time->tm_sec);
        return buf;
    }
    static TimeStamp now(){return TimeStamp(time(NULL));}
private:
    int64_t microSecondsSinceEpoch_;
};