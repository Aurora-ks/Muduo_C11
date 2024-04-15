#include "net/http/HttpServer.h"
#include "net/http/HttpResponse.h"
#include "net/http/HttpRequest.h"
#include "base/Logger.h"
#include <format>

using namespace http;
using namespace std::placeholders;

void DefaultHttpCallback(const HttpRequest &req, HttpResponse *res)
{
    res->SetStateCode(StateCode::NotFound);
    res->SetSateMessage("Not Found");
    res->SetKeepAlive(false);
}

HttpServer::HttpServer(EventLoop *loop, const Address &ListenAddr, const std::string &name, bool ReusePort)
    : server_(loop, ListenAddr, name, ReusePort),
      HttpCallback_(DefaultHttpCallback)
{
    server_.SetMessageCallback(
        std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
}

void HttpServer::SetThreadNum(int num)
{
    server_.SetThreadNum(num);
}

void HttpServer::start()
{
    LOG_DEBUG << std::format("HttpServer [{}] start on {}", server_.name(), server_.IpPort());
    server_.start();
}

void HttpServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp ReceiveTime)
{
    HttpRequest request;
    if(!request.parse(buf, ReceiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if(request.acceptable())
    {
        OnRequest(conn, request);
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn, const HttpRequest req)
{
    HttpResponse res;
    HttpCallback_(req, &res);
    Buffer buf;
    res.response(&buf);
    conn->send(&buf);
    if(!res.KeepAlive()) conn->shutdown();
}