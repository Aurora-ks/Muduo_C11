#include "base/Timestamp.h"
#include <sys/time.h>
#include <time.h>

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t secondes = tv.tv_sec;
    return Timestamp(secondes*1000 + tv.tv_usec);
}

std::string Timestamp::toString() const
{
    char buf[64]{0};
    time_t seconds = microSecondsSinceEpoch_ / 1000;
    auto time = localtime(&seconds);
    snprintf(buf, 64, "%4d/%02d/%02d %02d:%02d:%02d",
             time->tm_year + 1900,
             time->tm_mon + 1,
             time->tm_mday,
             time->tm_hour,
             time->tm_min,
             time->tm_sec);
    return buf;
}