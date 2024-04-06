#pragma once

#include "nocopyable.h"

class Address;

class Socket : nocopyable
{
public:
    explicit Socket(int fd) : sockfd_(fd) {}
    ~Socket();
    int fd() const { return sockfd_; }
    void BindAddress(const Address& localaddr);
    void listen();
    int accept(Address* peeraddr);

    void ShutdownWrite();

    void SetTcpNoDelay(bool on);
    void SetReuseAddr(bool on);
    void SetReusePort(bool on);
    void SetKeepAlive(bool on);
private:
    int sockfd_;
};