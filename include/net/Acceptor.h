#pragma once

#include <functional>
#include "nocopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class Address;

class Acceptor : nocopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const Address &address)>;
    Acceptor(EventLoop *loop, const Address &ListenAddress, bool reuseport);
    ~Acceptor();

    const Socket& socket() const { return sock_; }
    void SetNewConnectionCallback(const NewConnectionCallback &callback) { NewConnectionCallback_ = std::move(callback); }
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