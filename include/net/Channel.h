#pragma once

#include <functional>
#include <poll.h>
#include <memory>
#include "nocopyable.h"

class EventLoop;
class TimeStamp;
class TcpConnection;
class Channel : nocopyable
{
public:
    using ReadEventCallback = std::function<void(TimeStamp)>;
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd);
    ~Channel() = default;

    void SetReadCallback(const ReadEventCallback &f) { ReadCallback_ = std::move(f); }
    void SetWriteCallback(const EventCallback &f) { WriteCallback_ = std::move(f); }
    void SetCloseCallback(const EventCallback &f) { CloseCallback_ = std::move(f); }
    void SetErrorCallback(const EventCallback &f) { ErrorCallback_ = std::move(f); }
    void HandleEvent(TimeStamp time);

    // 防止channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

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

    bool isWriting() const { return events_ & POLLOUT; }
    bool isReading() const { return events_ & (POLLIN | POLLPRI); }
    bool isNoneEvent() const { return events_ == 0; }

    EventLoop *loop() { return loop_; }

    void remove();
private:
    void update();
    void HandleEventWithGuard(TimeStamp ReceiveTime);

    EventLoop *loop_;
    const int fd_;
    int events_;
    int revents_;
    int state_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback ReadCallback_;
    EventCallback WriteCallback_;
    EventCallback CloseCallback_;
    EventCallback ErrorCallback_;
};