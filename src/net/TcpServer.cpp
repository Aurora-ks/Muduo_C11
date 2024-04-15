#include "net/TcpServer.h"
#include "base/Logger.h"
#include "net/TcpConnection.h"
#include <functional>
#include <strings.h>
#include <format>

TcpServer::TcpServer(EventLoop *loop, const Address &ListenAddr, const std::string &name, bool ReusePort)
    : loop_(loop),
      IpPort_(ListenAddr.IpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, ListenAddr, ReusePort)),
      ThreadPool_(std::make_shared<EventLoopThreadPool>(loop, name)),
      ConnectionCallback_(),
      MessageCallback_(),
      NextConnectionId_(1)
{
    if (loop_ == nullptr)
    {
        LOG_FATAL << std::format("TcpServer::ctor loop is null");
    }
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    LOG_DEBUG << std::format("TcpServer::~TcpServer [{}] destructing", name_);
    for (auto &con : connections_)
    {
        TcpConnectionPtr conn(con.second);
        con.second.reset();
        conn->loop()->RunInLoop(std::bind(&TcpConnection::ConnectDestoryed, conn));
    }
}

void TcpServer::SetThreadNum(int num)
{
    ThreadPool_->SetThreadNum(num);
}

void TcpServer::start()
{
    LOG_INFO << std::format("TcpServer running on {}", IpPort_);
    if(started_++ == 0)
    {
        ThreadPool_->start(ThreadInitCallback_);
        loop_->RunInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::NewConnection(int sockfd, const Address &PeerAddrest)
{
    // 轮询选择subloop
    EventLoop *ioloop = ThreadPool_->GetNextLoop();
    char buf[64]{0};
    snprintf(buf, sizeof(buf), "-%s#%d", IpPort_.c_str(), NextConnectionId_);
    NextConnectionId_++;
    std::string name = name_ + buf;

    LOG_INFO << std::format("new connection [{}] from {}", name, PeerAddrest.IpPort());

    sockaddr_in local;
    bzero(&local, sizeof(local));
    socklen_t len = sizeof(local);
    if (::getsockname(acceptor_->socket().fd(), (sockaddr *)&local, &len) < 0)
    {
        LOG_ERROR << "TcpServer::NewConnection::getsockname";
    }
    Address LocalAddress(local);
    TcpConnectionPtr connection(new TcpConnection(ioloop, name, sockfd, LocalAddress, PeerAddrest));

    connections_[name] = connection;
    connection->SetConnectionCallback(ConnectionCallback_);
    connection->SetMessageCallback(MessageCallback_);
    connection->SetWriteCompleteCallback(WriteCompleteCallback_);
    connection->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

    ioloop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, connection));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &connection)
{
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, connection));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &connection)
{
    LOG_INFO << std::format("[{}] remove connection [{}]", name_, connection->name());

    connections_.erase(connection->name());
    EventLoop *loop = connection->loop();
    loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestoryed, connection));
}