#pragma once

#include "net/TcpServer.h"
#include "net/http/HttpHead.h"
#include "net/http/HttpRequest.h"
#include "net/http/HttpResponse.h"
#include <functional>

class HttpRequest;
class HttpResponse;

class HttpServer : nocopyable, public std::enable_shared_from_this<HttpServer>
{
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop, const Address &ListenAddr, const std::string &name, bool ReusePort = 1);

    void SetHttpCallback(const HttpCallback& f) { HttpCallback_ = std::move(f); }
    void SetThreadNum(int num);
    void start();

private:
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp ReceiveTime);
    void OnRequest(const TcpConnectionPtr& conn, const HttpRequest req);

    TcpServer server_;
    HttpCallback HttpCallback_;
};