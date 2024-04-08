#pragma once

#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <strings.h>

class Address
{
public:
    explicit Address() = default;
    explicit Address(std::string ip, uint16_t port);
    explicit Address(const sockaddr_in &addr): addr_(addr){}
    uint16_t port() const { return ntohs(addr_.sin_port); }
    std::string address() const;
    std::string IpPort() const;
    const sockaddr_in* sockaddr() const { return &addr_; }
    void SetSockaddr(const sockaddr_in &addr) { addr_ = addr; }
    ~Address() = default;
private:
    sockaddr_in addr_;
};