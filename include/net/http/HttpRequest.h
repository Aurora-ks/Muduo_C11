#pragma once

#include "HttpHead.h"
#include "base/Timestamp.h"
#include <string>
#include <unordered_map>

class Buffer;

class HttpRequest
{
public:
    enum ParseState
    {
        kParseLine,
        kParseHeader,
        kdown
    };

    HttpRequest() : method_(http::method::Unknown), version_(http::version::http11), state_(kParseLine) {}

    void SetMethod(http::method m) { method_ = m; }
    void SetMethod(const std::string& m);
    void SetVersion(http::version v) { version_ = v; }
    void SetUrl(const std::string url) { url_ = url; }
    void SetUrl(const char *begin, const char *end) { url_.assign(begin, end); }
    void SetQuery(const std::string q) { query_ = q; }
    void SetQuery(const char *begin, const char *end) { query_.assign(begin, end); }
    void SetReceiveTime(const Timestamp &t) { ReceiveTime_ = t; }

    http::method method() const { return method_; }
    http::version version() const { return version_; }
    std::string url() const { return url_; }
    std::string query() const { return query_; }
    Timestamp ReceiveTime() const { return ReceiveTime_; }

    void AddHeader(const std::string &key, const std::string &val);
    void AddHeader(const char *start, const char *colon, const char *end);
    std::string header(const std::string &key);
    const std::unordered_map<std::string, std::string>& headers() const { return headers_; }

    bool parse(Buffer* bufer, Timestamp ReceiveTime);
    bool acceptable() const { return state_ == kdown; }

    void swap(HttpRequest& other)
    {
        std::swap(method_, other.method_);
        std::swap(version_, other.version_);
        url_.swap(other.url_);
        query_.swap(other.query_);
        ReceiveTime_.swap(other.ReceiveTime_);
        headers_.swap(other.headers_);
    }

    // for debug
    const char* MethodToString() const;
    const char* VersionToString() const;

private:
    bool ParseLine(const char *begin, const char *end);

    http::method method_;
    http::version version_;
    std::string url_;
    std::string query_;
    Timestamp ReceiveTime_;
    ParseState state_;
    std::unordered_map<std::string, std::string> headers_;
};