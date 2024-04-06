#pragma once

#include "nocopyable.h"

class Socket : nocopyable
{
public:
    explicit Socket(int fd) : sockfd_(fd) {}
    ~Socket();
    int fd() const { return sockfd_; }


private:
    int sockfd_;
};