#pragma once

#include <functional>
#include "nocopyable.h"
#include "socket.h"
#include "channel.h"

class EventLoop;
class Address;

class Acceptor : nocopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const Address &address)>;
    Acceptor(EventLoop *loop, const Address &ListenAddress, bool reuseport);
    ~Acceptor();

    void SetNewConnectionCallback(NewConnectionCallback &callback) { NewConnectionCallback_ = std::move(callback); }
    bool listenning() const { return listenning_; }
    void listen();
private:
    void HandleRead();

    EventLoop *loop_; // 使用baseloop
    Socket sock_;
    Channel channel_;
    NewConnectionCallback NewConnectionCallback_;
    bool listenning_;
};