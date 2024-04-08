#pragma once

#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include "nocopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"
#include "Address.h"
#include "Callbacks.h"

class TcpServer : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    TcpServer(EventLoop* loop, const Address &ListenAddr, const std::string &name, bool ReusePort = 1);
    ~TcpServer();

    void SetThreadInitCallback(const ThreadInitCallback &callback) { ThreadInitCallback_ = std::move(callback); }
    void SetConnectionCallback(const ConnectionCallback &callback) { ConnectionCallback_ = std::move(callback); }
    void SetMessageCallback(const MessageCallback &callback) { MessageCallback_ = std::move(callback); }
    void SetWriteCompleteCallback(const WriteCompleteCallback &callback) { WriteCompleteCallback_ = std::move(callback); }

    void SetThreadNum(int num);
    void start();

private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    void NewConnection(int sockfd, const Address &PeerAddrest);
    void RemoveConnection(const TcpConnectionPtr &connection);
    void RemoveConnectionInLoop(const TcpConnectionPtr &connection);

    EventLoop* loop_;
    const std::string IpPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> ThreadPool_;

    ConnectionCallback ConnectionCallback_; // 有新连接的回调
    MessageCallback MessageCallback_; // 有数据读写时的回调
    WriteCompleteCallback WriteCompleteCallback_; // 数据发送完的回调

    ThreadInitCallback ThreadInitCallback_; // 线程初始化回调

    // std::atomic_int started_;
    int NextConnectionId_;
    ConnectionMap connections_; // 保存所有连接
};