#pragma once

#include <mutex>
#include <condition_variable>
#include <memory>
#include "base/nocopyable.h"

template <typename T>
class ThreadSafeQueue : nocopyable
{
public:
    ThreadSafeQueue() : head_(new node), tail_(head_.get()) {}
    ~ThreadSafeQueue() { cond_.notify_all(); }

    std::shared_ptr<T> WaitPop()
    {
        std::unique_ptr<node> const OldHead = WaitPopHead();
        return OldHead->data;
    }
    void WaitPop(T &value)
    {
        std::unique_ptr<node> const OldHead = WaitPopHead(value);
    }
    std::shared_ptr<T> TryPop()
    {
        std::unique_ptr<node> OldHead = TryPopHead();
        return OldHead ? OldHead->data : std::shared_ptr<T>();
    }
    bool TryPop(T &value)
    {
        std::unique_ptr<node> const OldHead = TryPopHead(value);
        return OldHead;
    }
    bool empty()
    {
        std::lock_guard<std::mutex> HeadLock(HeadMutex_);
        return (head_.get() == tail());
    }
    void push(T new_value)
    {
        std::shared_ptr<T> NewData(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node *const NewTail = p.get();
        std::lock_guard<std::mutex> TailLock(TailMutex);
        tail_->data = NewData;
        tail_->next = std::move(p);
        tail_ = NewTail;
        cond_.notify_one();
    }

    void NotifyAll() { cond_.notify_all(); }
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    node *tail()
    {
        std::lock_guard<std::mutex> TailLock(TailMutex);
        return tail_;
    }
    std::unique_ptr<node> PopHead()
    {
        std::unique_ptr<node> OldHead = std::move(head_);
        head_ = std::move(OldHead->next);
        return OldHead;
    }
    std::unique_lock<std::mutex> WaitData()
    {
        std::unique_lock<std::mutex> HeadLock(HeadMutex_);
        cond_.wait(HeadLock, [&]{ return head_.get() != tail(); });
        return std::move(HeadLock);
    }
    std::unique_ptr<node> WaitPopHead()
    {
        std::unique_lock<std::mutex> HeadLock(WaitData());
        return PopHead();
    }
    std::unique_ptr<node> WaitPopHead(T &value)
    {
        std::unique_lock<std::mutex> HeadLock(WaitData());
        value = std::move(*head_->data);
        return PopHead();
    }
    std::unique_ptr<node> TryPopHead()
    {
        std::lock_guard<std::mutex> HeadLock(HeadMutex_);
        if (head_.get() == tail())
        {
            return std::unique_ptr<node>();
        }
        return PopHead();
    }
    std::unique_ptr<node> TryPopHead(T &value)
    {
        std::lock_guard<std::mutex> HeadLock(HeadMutex_);
        if (head_.get() == tail())
        {
            return std::unique_ptr<node>();
        }
        value = std::move(*head_->data);
        return PopHead();
    }

    std::unique_ptr<node> head_;
    node *tail_;
    std::mutex HeadMutex_;
    std::mutex TailMutex;
    std::condition_variable cond_;
};