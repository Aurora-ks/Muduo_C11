#pragma once

#include <functional>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include "nocopyable.h"


class Thread : nocopyable
{
public:
    using TheadFun = std::function<void()>;

    explicit Thread(TheadFun fun, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    std::thread::id tid() const { return ThreadId_; }
    const std::string name() const { return name_; }

    static int32_t CreatCount() { return ThreadCount_.load(); }
    static std::string CurrentThreadId();

private:
    void SetDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    std::thread::id ThreadId_;
    TheadFun fun_;
    std::string name_;

    static std::atomic_int32_t ThreadCount_;
};