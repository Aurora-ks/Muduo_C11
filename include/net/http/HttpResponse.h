#pragma once

#include "net/http/HttpHead.h"
#include <unordered_map>
#include <string>

using namespace http;
class Buffer;

class HttpResponse
{
public:
    explicit HttpResponse()
        : version_(http::version::http11),
          StateCode_(static_cast<int>(StateCode::Unknown))
    {
        SetKeepAlive();
    }

    void SetVersion(http::version v) { version_ = v; }
    void SetStateCode(StateCode code) { StateCode_ = static_cast<int>(code); }
    void SetStateCode(int code) { StateCode_ = code; }
    void SetSateMessage(const std::string &msg) { StateMessage_ = msg; }
    void SetBody(const std::string &body) { body_ = body; }

    http::version version() const { return version_; }
    int StateCode() const { return StateCode_; }
    std::string StateMessage() const { return StateMessage_; }
    const std::unordered_map<std::string, std::string> &headers() const { return headers_; }
    std::string body() const { return body_; }

    void SetHeader(const std::string &k, const std::string &v)
    {
        headers_[k] = v;
    }

    void response(Buffer *output);

    void SetKeepAlive(bool on = true);
    bool KeepAlive() const;
    void SetContentLength(size_t len);
private:
    const char* VersionToString() const;

    http::version version_;
    int StateCode_;
    std::string StateMessage_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
};