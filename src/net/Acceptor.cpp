#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "Acceptor.h"
#include "Logger.h"
#include "Address.h"

static int CreateSocket()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d accept socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno)
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
    if (connfd > 0)
    {
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
        LOG_ERROR("%s:%s:%d accept error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno)
    }
}