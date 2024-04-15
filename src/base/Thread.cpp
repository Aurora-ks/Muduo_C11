#include "base/Thread.h"
#include <latch>
#include <sstream>

std::atomic_int32_t Thread::ThreadCount_(0);

Thread::Thread(TheadFun fun, const std::string &name)
    : started_(false),
      joined_(false),
      ThreadId_(0),
      fun_(std::move(fun)),
      name_(name)
{
    SetDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    std::latch latch(1);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        ThreadId_ = std::this_thread::get_id();
        latch.count_down();
        fun_();
    }));
    //确保ThreadId_创建完毕
    latch.wait();
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::SetDefaultName()
{
    int num = ++ThreadCount_;
    if(name_.empty())
    {
        char buf[32]{0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

std::string Thread::CurrentThreadId()
{
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}