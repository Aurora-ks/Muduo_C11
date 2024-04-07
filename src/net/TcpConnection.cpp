#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"
#include <functional>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/tcp.h>

TcpConnection::TcpConnection(EventLoop *loop, const std::string name, int sockfd, const Address &local, const Address &remote)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      LocalAddress_(local),
      RemoteAddress_(remote),
      HighWaterMark_(64 * 1024 * 1024) // 64M
{
    if(loop_ == nullptr)
    {
        LOG_FATAL("%s:%s:%d connectionloop is null\n", __FILE__, __FUNCTION__, __LINE__)
    }
    channel_->SetReadCallback(std::bind(&TcpConnection::HeadleRead, this, std::placeholders::_1));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG("TcpConnection::destruct[%s]-fd=%d-state=%d\n", name_.c_str(), channel_->fd(), state_.load())
}

void TcpConnection::send(const void *message, int len)
{
}
void TcpConnection::shutdown()
{
}
void TcpConnection::ConnectEstablished()
{
}
void TcpConnection::ConnectDestoryed()
{
}

void TcpConnection::HeadleRead(TimeStamp ReceiveTime)
{
    int err = 0;
    ssize_t n = InputBuffer_.ReadFromFd(socket_->fd(), &err);
    if(n > 0)
    {
        MessageCallback_(shared_from_this(), &InputBuffer_, ReceiveTime);
    }
    else if(n == 0)
    {
        HandleClose();
    }
    else
    {
        err = errno;
        LOG_ERROR("TcpConnection::HeadleRead")
        HandleError();
    }
}

void TcpConnection::HandleWrite()
{
    if(channel_->isWriting())
    {
        int err = 0;
        ssize_t n = OutputBuffer_.WriteFromFd(socket_->fd(), &err);
        if(n > 0)
        {
            OutputBuffer_.retrieve(n);
            if(OutputBuffer_.ReadableBytes() == 0)
            {
                channel_->DisableWriting();
                if(WriteCompleteCallback_)
                {
                    loop_->RunInLoop(std::bind(WriteCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting)
                {
                    ShutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::HandleWrite")
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd = %d is down, no more writing\n", socket_->fd())
    }
}

void TcpConnection::HandleClose()
{
    LOG_INFO("TcpConnection::HandleClose fd=%d close, state=%d\n", socket_->fd(), state_.load())
    state_ = kDisconnected;
    channel_->DisableAll();
    TcpConnectionPtr self = shared_from_this();
    if(ConnectionCallback_)
    {
        ConnectionCallback_(self);
    }
    if(CloseCallback_)
    {
        CloseCallback_(self);
    }
}

void TcpConnection::HandleError()
{
    int opt;
    socklen_t optlen = sizeof(opt);
    int err = 0;
    if(::getsockopt(socket_->fd(), SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = opt;
    }
    LOG_ERROR("TcpConnection::HandleError name:%s SO_ERROR:%d\n", name_.c_str(), err)
}

void TcpConnection::SendInLoop(const void *message, size_t len)
{
}
void TcpConnection::ShutdownInLoop()
{
}