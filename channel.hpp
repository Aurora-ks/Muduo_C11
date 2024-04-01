#pragma once

#include <functional>
#include <poll.h>
#include "nocopyable.h"
#include "timestamp.hpp"

class EventLoop;

class Channel : nocopyable
{
public:
    using ReadEventCallback = std::function<void(TimeStamp)>;
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd) : loop_(loop),
                                       fd_(fd),
                                       events_(0),
                                       revents_(0),
                                       state_(0) {}
    ~Channel() = default;

    void SetReadCallback(ReadEventCallback f) { ReadCallback_ = std::move(f); }
    void SetWriteCallback(EventCallback f) { WriteCallback_ = std::move(f); }
    void SetCloseCallback(EventCallback f) { CloseCallback_ = std::move(f); }
    void setErrorCallback(EventCallback f) { ErrorCallback_ = std::move(f); }
    void HandleEvent(TimeStamp time)
    {
        if (revents_ & POLLHUP && !(revents_ & POLLIN))
        {
            if (CloseCallback_)
                CloseCallback_();
        }
        if (revents_ & POLLERR)
        {
            if (ErrorCallback_)
                ErrorCallback_();
        }
        if (revents_ & (POLLIN | POLLPRI))
        {
            if (ReadCallback_)
                ReadCallback_(time);
        }
        if (revents_ & POLLOUT)
        {
            if (WriteCallback_)
                WriteCallback_();
        }
    }

    int events() const { return events_; }
    int fd() const { return fd_; }
    int revents() const { return revents_; }
    void SetRevents(int revents) { revents_ = revents; }
    int state() const { return state_; }
    void SetState(int state) { state_ = state; }

    void EnableReading()
    {
        events_ |= (POLLIN | POLLPRI);
        update();
    }

    void DisableReading()
    {
        events_ &= ~(POLLIN | POLLPRI);
        update();
    }

    void EnableWriting()
    {
        events_ |= POLLOUT;
        update();
    }

    void DisableWriting()
    {
        events_ &= ~POLLOUT;
        update();
    }

    void DisableAll()
    {
        events_ = 0;
        update();
    }

    bool isWriting() const { return events_ & (POLLIN | POLLPRI); }
    bool isReading() const { return events_ & POLLOUT; }
    bool isNoneEvent() const { return events == 0; }

    EventLoop *loop() { return loop_; }

private:
    void update()
    {
    }
    void remove()
    {
    }

    EventLoop *loop_;
    const int fd_;
    int events_;
    int revents_;
    int state_;
    ReadEventCallback ReadCallback_;
    EventCallback WriteCallback_;
    EventCallback CloseCallback_;
    EventCallback ErrorCallback_;
};