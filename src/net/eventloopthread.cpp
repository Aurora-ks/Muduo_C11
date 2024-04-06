#include "eventloopthread.h"
#include "eventloop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &callback, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunciton, this), name),
      mutex_(),
      cond_(),
      callback_(callback)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::StartLoop()
{
    //启动线程执行ThreadFunciton
    thread_.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&]{return loop_ != nullptr;});
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::ThreadFunciton()
{
    EventLoop loop;
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.start();
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}