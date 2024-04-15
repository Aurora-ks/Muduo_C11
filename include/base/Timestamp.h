#pragma once

#include <string>
#include <time.h>
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
    static Timestamp now(){return Timestamp(time(NULL));}
private:
    int64_t microSecondsSinceEpoch_;
};