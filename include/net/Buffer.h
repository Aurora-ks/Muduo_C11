#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t InitialSize = kInitialSize)
        : buffer_(kCheapPrepend + InitialSize),
          ReaderIndex_(kCheapPrepend),
          WriterIndex_(kCheapPrepend)
    {}

    size_t ReadableBytes() const { return WriterIndex_ - ReaderIndex_; }
    size_t WriteableBytes() const { return buffer_.size() - WriterIndex_; }
    size_t PrependBytes() const { return ReaderIndex_; }

    // 返回缓冲区可读段起始地址
    const char* peek() const { return begin() + ReaderIndex_; }

    void retrieve(size_t len);
    void RetrieveAll();
    std::string RetrieveToString(size_t len);
    std::string RetrieveAllToString();
    void EnsureWriteableBytes(size_t len);

    //写入数据
    void append(const char* data, size_t len);
    ssize_t ReadFromFd(int fd, int* err);
private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    char* BeginWrite() { return begin() + WriterIndex_; }
    const char* BeginWrite() const { return begin() + WriterIndex_; }

    //buffer扩容
    void MakeSpace(size_t len);

    std::vector<char> buffer_;
    size_t ReaderIndex_;
    size_t WriterIndex_;
};