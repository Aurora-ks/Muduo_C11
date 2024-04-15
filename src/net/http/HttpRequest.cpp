#include "net/http/HttpRequest.h"
#include "net/Buffer.h"

using namespace http;
using namespace std;

void HttpRequest::SetMethod(const string &m)
{
    if (m == "GET")
        method_ = method::GET;
    else if (m == "HEAD")
        method_ = method::HEAD;
    else if (m == "POST")
        method_ = method::POST;
    else if (m == "PUT")
        method_ = method::PUT;
    else if (m == "DELETE")
        method_ = method::DELETE;
    else
        method_ = method::Unknown;
}

void HttpRequest::AddHeader(const string &k, const string &v)
{
    headers_[k] = v;
}

void HttpRequest::AddHeader(const char *start, const char *colon, const char *end)
{
    string key(start, colon);
    ++colon;
    while(colon < end && isspace(*colon)) ++ colon;
    while(end > colon && isspace(*end)) --end;
    string value(colon, end);
    headers_[key] = value;
}

string HttpRequest::header(const string &filed)
{
    auto i = headers_.find(filed);
    if(i == headers_.end()) return "";
    return i->second;
}

bool HttpRequest::ParseLine(const char *begin, const char *end)
{
    bool success = false;
    const char *start = begin;
    const char *space = find(start, end, ' ');
    if(space != end)
    {
        SetMethod(string(start, space));
        if(method_ == method::Unknown) return success;

        start = space + 1;
        space = find(start, end, ' ');
        if(space != end)
        {
            const char *question = find(start, space, '?');
            if(question != space)
            {
                SetUrl(start, question);
                SetQuery(question, space);
            }
            else SetUrl(start, space);

            start = space + 1;
            success = end-start == 8 && equal(start, end-3, "HTTP/");
            if(success)
            {
                if(*(end-3) == '2') SetVersion(version::http20);
                else if(*(end-1) == '0') SetVersion(version::http10);
                else if(*(end-1) == '1') SetVersion(version::http11);
                else success = false;
            }
        }
    }
    return success;
}

bool HttpRequest::parse(Buffer* buffer, Timestamp ReceiveTime)
{
    bool ok = true;
    bool parsing = true;
    while(parsing)
    {
        if(state_ == kParseLine)
        {
            const char *crlf = buffer->findCRLF();
            if(crlf)
            {
                ok = ParseLine(buffer->peek(), crlf);
                if(ok)
                {
                    SetReceiveTime(ReceiveTime);
                    buffer->RetrieveUntil(crlf + 2);
                    state_ = kParseHeader;
                }
                else parsing = false;
            }
            else parsing = false;
        }
        else if(state_ == kParseHeader)
        {
            const char *crlf = buffer->findCRLF();
            if(crlf)
            {
                const char *colon = find(buffer->peek(), crlf, ':');
                if(colon != crlf)
                {
                    AddHeader(buffer->peek(), colon, crlf);
                }
                else
                {
                    state_ = kdown;
                    parsing = false;
                }
                buffer->RetrieveUntil(crlf + 2);
            }
            else parsing = false;
        }
        else if(state_ == kdown) parsing = false;
    }
    return ok;
}

const char* HttpRequest::MethodToString() const
{
    switch (method_)
    {
    case method::GET:
        return "GET";
    case method::HEAD:
        return "HEAD";
    case method::POST:
        return "POST";
    case method::PUT:
        return "PUT";
    case method::DELETE:
        return "DELETE";
    default:
        return "Unknown";
    }
}

const char *HttpRequest::VersionToString() const
{
    switch (version_)
    {
    case version::http10:
        return "HTTP 1.0";
    case version::http11:
        return "HTTP 1.1";
    case version::http20:
        return "HTTP 2.0";
    default:
        return "Unknown";
    }
}