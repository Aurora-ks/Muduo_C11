#include <memory>
#include "eventloopthreadpool.h"
#include "eventloopthread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *BaseLoop, const std::string &name)
    : BaseLoop_(BaseLoop),
      name_(name),
      started_(false),
      ThreadNum_(0),
      next_(0)
{}

void EventLoopThreadPool::start(const ThreadInitCallback &callback)
{
    started_ = true;
    for(int i = 0; i < ThreadNum_; i++)
    {
        char buf[name_.size()+32]{0};
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(callback, buf);
        threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
        loops_.emplace_back(t->StartLoop());
    }
    if(ThreadNum_ == 0 && callback)
    {
        callback(BaseLoop_);
    }
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
    EventLoop *loop = BaseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_++;
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::AllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, BaseLoop_);
    }
    else
    {
        return loops_;
    }
}