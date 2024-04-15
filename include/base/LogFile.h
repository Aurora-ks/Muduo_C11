#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include "base/nocopyable.h"

class LogFile : nocopyable
{
public:
    LogFile(const std::string &name, int roll, int flush = 5, int check = 1024);
    ~LogFile() = default;

    void append(const char *msg, int len);
    void flush();
    bool RollFile();

private:
    std::string LogFileName(const std::string &name, time_t *now);

    const std::string basename_;
    const int RollSize_;
    const int FlushInterval_;
    const int CheckEveryN_;
    int count_;

    std::mutex mutex_;
    time_t start_;
    time_t LastRoll_;
    time_t LastFlush_;
    std::ofstream file_;
    std::string filename_;

    const static int kRollPerSeconds_ = 60*60*24; // 1 day
};