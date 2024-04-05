#include "address.h"

Addrest::Addrest(std::string ip, uint16_t port)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

std::string Addrest::address() const
{
    char buf[64]{0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

std::string Addrest::IpPort() const
{
    char buf[64]{0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    int len = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + len, ":%u", port);
    return std::string(buf);
}