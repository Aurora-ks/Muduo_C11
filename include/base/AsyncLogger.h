#pragma once

#include "base/nocopyable.h"
#include "base/LogStream.h"
#include "base/Thread.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <condition_variable>
#include <latch>

class AsyncLogger : nocopyable
{
public:
    AsyncLogger(const std::string& name, int roll, int flush = 5);
    ~AsyncLogger();

    void append(const char* msg, int len);
    void start();
    void stop();

private:
    void StartLog();

    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

    const int Flushinterval_;
    std::atomic_bool running_;
    const std::string basename_;
    const int RollSize_;
    Thread thread_;
    std::latch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr CurrentBuffer_;
    BufferPtr NextBuffer_;
    BufferVector buffers_;
};