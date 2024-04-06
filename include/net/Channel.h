#pragma once

#include <functional>
#include <poll.h>
#include "nocopyable.h"

class EventLoop;
class TimeStamp;

class Channel : nocopyable
{
public:
    using ReadEventCallback = std::function<void(TimeStamp)>;
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd);
    ~Channel() = default;

    void SetReadCallback(ReadEventCallback f) { ReadCallback_ = std::move(f); }
    void SetWriteCallback(EventCallback f) { WriteCallback_ = std::move(f); }
    void SetCloseCallback(EventCallback f) { CloseCallback_ = std::move(f); }
    void setErrorCallback(EventCallback f) { ErrorCallback_ = std::move(f); }
    void HandleEvent(TimeStamp time);

    int events() const { return events_; }
    int fd() const { return fd_; }
    int revents() const { return revents_; }
    void SetRevents(int revents) { revents_ = revents; }
    int state() const { return state_; }
    void SetState(int state) { state_ = state; }

    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();
    void DisableAll();

    bool isWriting() const { return events_ & (POLLIN | POLLPRI); }
    bool isReading() const { return events_ & POLLOUT; }
    bool isNoneEvent() const { return events_ == 0; }

    EventLoop *loop() { return loop_; }

    void remove();
private:
    void update();

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