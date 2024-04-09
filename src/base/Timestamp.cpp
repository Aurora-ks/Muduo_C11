#include "Timestamp.h"

std::string Timestamp::toString() const
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