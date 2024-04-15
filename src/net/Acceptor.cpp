#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <format>
#include "net/Acceptor.h"
#include "net/Address.h"
#include "base/Logger.h"

static int CreateSocket()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL << std::format("accept socket create error:{}", errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const Address &ListenAddress, bool reuseport)
    : loop_(loop),
      sock_(CreateSocket()),
      channel_(loop, sock_.fd()),
      listenning_(false)
{
    sock_.SetReuseAddr(true);
    sock_.SetReusePort(reuseport);
    sock_.BindAddress(ListenAddress);
    channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor()
{
    channel_.DisableAll();
    channel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    sock_.listen();
    channel_.EnableReading();
}

void Acceptor::HandleRead()
{
    Address peeraddr;
    int connfd = sock_.accept(&peeraddr);
    if (connfd >= 0)
    {
        LOG_DEBUG << std::format("accept from {} fd = {}", peeraddr.IpPort(), connfd);
        if (NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd, peeraddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR << "in Acceptor::HandleRead";
    }
}