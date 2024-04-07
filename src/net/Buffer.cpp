#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

void Buffer::retrieve(size_t len)
{
    if (len < ReadableBytes())
    {
        ReaderIndex_ += len;
    }
    else
    {
        RetrieveAll();
    }
}

void Buffer::RetrieveAll()
{
    ReaderIndex_ = WriterIndex_ = kCheapPrepend;
}

std::string Buffer::RetrieveToString(size_t len)
{
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::RetrieveAllToString()
{
    return RetrieveToString(ReadableBytes());
}

void Buffer::EnsureWriteableBytes(size_t len)
{
    if (WriteableBytes() < len)
    {
        MakeSpace(len);
    }
}

// 写入数据
void Buffer::append(const char *data, size_t len)
{
    EnsureWriteableBytes(len);
    std::copy(data, data + len, BeginWrite());
    WriterIndex_ += len;
}

ssize_t Buffer::ReadFromFd(int fd, int *err)
{
    char extrabuf[65536]{0}; // 64K
    struct iovec vec[2];
    const size_t writeable = WriteableBytes();
    vec[0].iov_base = begin() + WriterIndex_;
    vec[0].iov_len = writeable;
    vec[0].iov_base = extrabuf;
    vec[0].iov_len = sizeof(extrabuf);
    const int iovcnt = (writeable < sizeof(extrabuf) ? 2 : 1);
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *err = errno;
    }
    else if(n <= writeable)
    {
        WriterIndex_ += n;
    }
    else
    {
        WriterIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }
    return n;
}

void Buffer::MakeSpace(size_t len)
{
    if (WriteableBytes() + PrependBytes() < len + kCheapPrepend)
    {
        buffer_.resize(WriteableBytes() + len);
    }
    else
    {
        size_t DataLen = ReadableBytes();
        std::copy(begin() + ReaderIndex_, begin() + WriterIndex_, begin() + kCheapPrepend);
        ReaderIndex_ = kCheapPrepend;
        WriterIndex_ = ReaderIndex_ + DataLen;
    }
}