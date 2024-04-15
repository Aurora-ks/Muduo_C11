#include "net/http/HttpResponse.h"
#include "net/Buffer.h"
#include <format>

using namespace std;

void HttpResponse::response(Buffer *output)
{
    string prot = format("{} {} {}\r\n",VersionToString(), StateCode_, StateMessage_);
    output->append(prot);

    SetHeader("Content-Length", to_string(body_.size()));

    for(const auto& s : headers_)
    {
        output->append(s.first);
        output->append(": ");
        output->append(s.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    if(!body_.empty()) output->append(body_);
}

void HttpResponse::SetKeepAlive(bool on)
{
    headers_["Connection"] = on ? "Keep-Alive" : "close";
}

bool HttpResponse::KeepAlive() const
{
    if(headers_.at("Connection") == "close") return false;
    else return true;
}

void HttpResponse::SetContentLength(size_t len)
{
    headers_["Content-Length"] = len;
}

const char* HttpResponse::VersionToString() const
{
    if(version_ == version::http10) return "HTTP/1.0";
    if(version_ == version::http11) return "HTTP/1.1";
    else return "HTTP/2.0";
}