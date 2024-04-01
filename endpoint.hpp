#pragma once

#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <strings.h>

class EndPoint
{
public:
    explicit EndPoint(std::string ip, uint16_t port)
    {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
        addr_.sin_port = htons(port);
    }
    uint16_t port() const { return ntohs(addr_.sin_port); }
    std::string address() const
    {
        char buf[64]{0};
        inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
        return std::string(buf);
    }
    std::string IpPort() const
    {
        char buf[64]{0};
        inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
        int len = strlen(buf);
        uint16_t port = ntohs(addr_.sin_port);
        sprintf(buf + len, ":%u", port);
        return std::string(buf);
    }

private:
    sockaddr_in addr_;
};