#include "TcpServer.h"
#include "Logger.h"
#include <functional>

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
        LOG_FATAL("%s:%s:%d mainloop is null\n", __FILE__, __FUNCTION__, __LINE__)
    }
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() = default;

void TcpServer::SetThreadNum(int num)
{
    ThreadPool_->SetThreadNum(num);
}

void TcpServer::start()
{
    if(started_++ == 0) // 防止start被多次调用
    {
        ThreadPool_->start(ThreadInitCallback_);
        loop_->RunInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::NewConnection(int sockfd, const Address &PeerAddrest)
{
}
void TcpServer::RemoveConnection(TcpConnectionPtr &connection)
{
}
void TcpServer::RemoveConnectionInLoop(TcpConnectionPtr &connection)
{
}