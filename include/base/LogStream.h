#pragma once

#include "nocopyable.h"
#include <strings.h>
#include <string.h>
#include <string>

const int kSmallBuffer = 4096;        // 4K for LogStream
const int kLargeBuffer = 4096 * 1024; // 4M for AsyncLogger

template <int SIZE>
class FixedBuffer : nocopyable
{
public:
    FixedBuffer() : cur_(data_){}

    void append(const char* msg, size_t len)
    {
        if(FreeSize() > len)
        {
            memcpy(cur_, msg, len);
            cur_ += len;
        }
    }

    void add(int len) { cur_ += len; }
    void reset() { cur_ = data_; }
    void bzero() { ::bzero(data_, sizeof(data_)); }
    size_t FreeSize() { return static_cast<size_t>(end() - cur_); }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }
    char* current() const { return cur_; }
    std::string toString() const { return std::string(data_, length()); }

private:
    const char* end() const { return data_ + sizeof(data_); }

    char data_[SIZE];
    char *cur_;
};

class LogStream : nocopyable
{
public:
    using Buffer = FixedBuffer<kSmallBuffer>;

    LogStream& operator << (bool v)
    {
        if(v) buffer_.append("true", 4);
        else buffer_.append("false", 5);
        return *this;
    }

    LogStream& operator << (unsigned short v)
    {
        *this << static_cast<unsigned int>(v);
        return *this;
    }
    LogStream& operator << (unsigned int v);
    LogStream& operator << (unsigned long v);
    LogStream& operator << (unsigned long long v);

    LogStream& operator << (short v)
    {
        *this << static_cast<int>(v);
        return *this;
    }
    LogStream& operator << (int v);
    LogStream& operator << (long v);
    LogStream& operator << (long long v);
    LogStream& operator << (float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator << (double v);

    LogStream& operator << (char v);
    LogStream& operator << (const char* v);
    LogStream& operator << (const std::string v);

    LogStream& operator << (const Buffer& v);

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void ResetBuffer() { buffer_.reset(); }

private:
    template<typename T>
    void FormatInteger(T);

    Buffer buffer_;
    static const int kMaxNumericSize = 32;
};