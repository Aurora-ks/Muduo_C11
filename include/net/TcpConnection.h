#pragma once

#include "../base/nocopyable.h"
#include "../base/Timestamp.h"
#include "Address.h"
#include "Callbacks.h"
#include "Buffer.h"
#include <memory>
#include <string>
#include <atomic>

class Channel;
class Socket;
class EventLoop;

class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    enum state { kDisconnected, kDisconnecting, kConnecting, kConnected };

    TcpConnection(EventLoop *loop, const std::string name, int sockfd, const Address &local, const Address &remote);
    ~TcpConnection();

    EventLoop* loop() const { return loop_; }
    const std::string& name() const { return name_; }
    const Address& LocalAddress() const { return LocalAddress_; }
    const Address& RemoteAddress() const { return RemoteAddress_; }
    bool connected() const {return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const std::string &data);
    void shutdown();

    void SetConnectionCallback(const ConnectionCallback &callback) { ConnectionCallback_ = std::move(callback); }
    void SetMessageCallback(const MessageCallback &callback) { MessageCallback_ = std::move(callback); }
    void SetWriteCompleteCallback(const WriteCompleteCallback &callback) { WriteCompleteCallback_ = std::move(callback); }
    void SetCloseCallback(const CloseCallback &callback) { CloseCallback_ = std::move(callback); }
    void SetHighWaterMarkCallback(const HighWaterMarkCallback &callback) { HighWaterMarkCallback_ = std::move(callback); }

    void ConnectEstablished();
    void ConnectDestoryed();
private:
    void HeadleRead(Timestamp ReceiveTime);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void SendInLoop(const void *data, size_t len);
    void ShutdownInLoop();

    EventLoop *loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const Address LocalAddress_;
    const Address RemoteAddress_;

    ConnectionCallback ConnectionCallback_; // 有新连接的回调
    MessageCallback MessageCallback_; // 有数据读写时的回调
    WriteCompleteCallback WriteCompleteCallback_; // 数据发送完的回调
    CloseCallback CloseCallback_; // 连接关闭回调
    HighWaterMarkCallback HighWaterMarkCallback_; // 数据发送过多时回调
    size_t HighWaterMark_;
    Buffer InputBuffer_;
    Buffer OutputBuffer_;
};