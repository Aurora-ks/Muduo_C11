#pragma once

#include <string>
#include <time.h>
#include <stdint.h>

class TimeStamp
{
public:
    TimeStamp(): microSecondsSinceEpoch_(0) {}
    explicit TimeStamp(int64_t time): microSecondsSinceEpoch_(time) {}
    std::string toString() const;
    static TimeStamp now(){return TimeStamp(time(NULL));}
private:
    int64_t microSecondsSinceEpoch_;
};