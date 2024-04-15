#include "base/LogStream.h"
#include <algorithm>

const char digits[] = "0123456789";
const char* zero = digits;

template<typename T>
size_t convert(char *buf, T v)
{
    T i = v;
    char *p = buf;

    do
    {
        int t = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[t];
    }while(i);

    if(v < 0) *p++ = '-';
    *p = '\0';

    std::reverse(buf, p);

    return p - buf;
}

template<typename T>
void LogStream::FormatInteger(T v)
{
    if(buffer_.FreeSize() > kMaxNumericSize)
    {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(unsigned int v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    FormatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if(buffer_.FreeSize() >= kMaxNumericSize)
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(char v)
{
    buffer_.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char *v)
{
    if(v)
    {
        buffer_.append(v, strlen(v));
    }
    else
    {
        buffer_.append("(null)", 6);
    }
    return *this;

}
LogStream& LogStream::operator<<(const std::string v)
{
    buffer_.append(v.c_str(), v.size());
    return *this;
}

LogStream& LogStream::operator<<(const Buffer &v)
{
    *this << v.toString();
    return *this;
}