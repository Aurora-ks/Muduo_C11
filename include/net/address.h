#pragma once

#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <strings.h>

class Addrest
{
public:
    explicit Addrest(std::string ip, uint16_t port);
    uint16_t port() const { return ntohs(addr_.sin_port); }
    std::string address() const;
    std::string IpPort() const;
    ~Addrest() = default;
private:
    sockaddr_in addr_;
};