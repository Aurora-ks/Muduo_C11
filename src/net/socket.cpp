#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/tcp.h>
#include "socket.h"
#include "address.h"
#include "logger.h"

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::BindAddress(const Address &localaddr)
{
    if (0 != ::bind(sockfd_, (sockaddr *)localaddr.sockaddr(), sizeof(struct sockaddr_in)))
    {
        LOG_FATAL("sockfd:%d bind failed\n", sockfd_)
    }
}

void Socket::listen()
{
    if (0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("sockfd:%d listen failed\n", sockfd_)
    }
}

int Socket::accept(Address *peeraddr)
{
    sockaddr_in addr;
    socklen_t len;
    bzero(&addr, sizeof(addr));
    int cfd = ::accept(sockfd_, (sockaddr *)&addr, &len);
    if (cfd >= 0)
    {
        peeraddr->SetSockaddr(addr);
    }
    return cfd;
}

void Socket::ShutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("sockfd:%d ShutdownWrite error\n", sockfd_)
    }
}

void Socket::SetTcpNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void Socket::SetReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::SetReusePort(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::SetKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}