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
#include <format>

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
    if (loop_ == nullptr)
    {
        LOG_FATAL << "TcpConnection::ctor loop is null";
    }
    channel_->SetReadCallback(std::bind(&TcpConnection::HeadleRead, this, std::placeholders::_1));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
    socket_->SetKeepAlive(true);
    LOG_DEBUG << std::format("TcpConnection::ctor[{}] at {} fd = {}",
                             name_, reinterpret_cast<unsigned long>(this), socket_->fd());
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << std::format("TcpConnection::dtor[{}] at {} fd = {} state = {}",
                             name_, reinterpret_cast<unsigned long>(this), socket_->fd(), StateToString());
}

void TcpConnection::send(const std::string &data)
{
    if (state_ == kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(data.c_str(), data.size());
        }
        else
        {
            loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, data.c_str(), data.size()));
        }
    }
}

void TcpConnection::SendInLoop(const void *data, size_t len)
{
    ssize_t nwrite = 0;
    size_t remain = len;
    bool err = false;
    // 调用过shutdown，不能发送
    if (state_ == kDisconnected)
    {
        LOG_WARN << std::format("connection fd = {} disconnected, give up writing", socket_->fd());
    }
    // channel第一次发数据，并且缓冲区没有可发送的数据
    if (!channel_->isWriting() && OutputBuffer_.ReadableBytes() == 0)
    {
        nwrite = ::write(channel_->fd(), data, len);
        if (nwrite >= 0)
        {
            remain = len - nwrite;
            if (remain == 0 && WriteCompleteCallback_)
            {
                loop_->QueueInLoop(std::bind(WriteCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrite = 0;
            if (errno != EAGAIN)
            {
                LOG_ERROR << "TcpConnection::SendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    err = true;
                }
            }
        }
    }
    if (!err && remain > 0)
    {
        // 缓冲区剩余数据
        size_t bufremain = OutputBuffer_.ReadableBytes();
        if (bufremain + remain >= HighWaterMark_ && bufremain < HighWaterMark_ && HighWaterMarkCallback_)
        {
            loop_->QueueInLoop(std::bind(HighWaterMarkCallback_, shared_from_this(), bufremain + remain));
        }
        OutputBuffer_.append((char *)data + nwrite, remain);
        if (!channel_->isWriting())
        {
            channel_->EnableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        state_ = kDisconnecting;
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop()
{
    // 没有数据需要发送
    if (!channel_->isWriting())
    {
        socket_->ShutdownWrite();
    }
}

void TcpConnection::ConnectEstablished()
{
    state_ = kConnected;
    channel_->tie(shared_from_this());
    channel_->EnableReading();
    if (ConnectionCallback_)
    {
        ConnectionCallback_(shared_from_this());
    }
}

void TcpConnection::ConnectDestoryed()
{
    if (state_ == kConnected)
    {
        state_ = kDisconnected;
        channel_->DisableAll();
        if (ConnectionCallback_)
        {
            ConnectionCallback_(shared_from_this());
        }
    }
    channel_->remove();
}

void TcpConnection::HeadleRead(Timestamp ReceiveTime)
{
    int err = 0;
    ssize_t n = InputBuffer_.ReadFromFd(socket_->fd(), &err);
    if (n > 0)
    {
        MessageCallback_(shared_from_this(), &InputBuffer_, ReceiveTime);
    }
    else if (n == 0)
    {
        HandleClose();
    }
    else
    {
        err = errno;
        LOG_ERROR << "TcpConnection::HeadleRead";
        HandleError();
    }
}

void TcpConnection::HandleWrite()
{
    if (channel_->isWriting())
    {
        int err = 0;
        ssize_t n = OutputBuffer_.WriteFromFd(socket_->fd(), &err);
        if (n > 0)
        {
            OutputBuffer_.retrieve(n);
            if (OutputBuffer_.ReadableBytes() == 0)
            {
                channel_->DisableWriting();
                if (WriteCompleteCallback_)
                {
                    loop_->QueueInLoop(std::bind(WriteCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    ShutdownInLoop();
                }
            }
        }
        else
            LOG_ERROR << "TcpConnection::HandleWrite";
    }
    else
        LOG_DEBUG << std::format("connection fd = {} is down, no more writing", socket_->fd());
}

void TcpConnection::HandleClose()
{
    LOG_DEBUG << std::format("fd = {} state = {} close", socket_->fd(), StateToString());
    state_ = kDisconnected;
    channel_->DisableAll();
    TcpConnectionPtr self = shared_from_this();
    if (ConnectionCallback_)
    {
        ConnectionCallback_(self);
    }
    if (CloseCallback_)
    {
        CloseCallback_(self);
    }
}

void TcpConnection::HandleError()
{
    int opt;
    socklen_t optlen = sizeof(opt);
    int err = 0;
    if (::getsockopt(socket_->fd(), SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0)
        err = errno;
    else
        err = opt;
    LOG_ERROR << std::format("TcpConnection::handleError [{}] - SO_ERROR = {}", name_, err);
}

const char *TcpConnection::StateToString() const
{
    switch (state_)
    {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisconnecting:
        return "kDisconnecting";
    default:
        return "unknown state";
    }
}