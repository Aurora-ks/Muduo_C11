#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"
#include <functional>
#include <strings.h>

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
        // LOG_FATAL("%s:%s:%d mainloop is null\n", __FILE__, __FUNCTION__, __LINE__)
    }
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
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

    // LOG_INFO("%s:%s:%d [%s] - new connection [%s] from %s\n", __FILE__, __FUNCTION__, __LINE__,
            //  name_.c_str(), name.c_str(), PeerAddrest.IpPort().c_str());

    sockaddr_in local;
    bzero(&local, sizeof(local));
    socklen_t len = sizeof(local);
    if (::getsockname(acceptor_->socket().fd(), (sockaddr *)&local, &len) < 0)
    {
        // LOG_ERROR("TcpServer::NewConnection::getsockname")
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
    // LOG_INFO("%s:%s:%d [%s] - remove connection [%s]\n", __FILE__, __FUNCTION__, __LINE__,
            //  name_.c_str(), connection->name().c_str());

    connections_.erase(connection->name());
    EventLoop *loop = connection->loop();
    loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestoryed, connection));
}