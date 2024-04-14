#include "AsyncLogger.h"
#include "LogFile.h"
#include "Timestamp.h"
#include <functional>
#include <semaphore.h>

AsyncLogger::AsyncLogger(const std::string &name, int roll, int flush)
    : Flushinterval_(flush),
      running_(false),
      basename_(name),
      RollSize_(roll),
      thread_(std::bind(&AsyncLogger::StartLog, this), "Logging"),
      latch_(1),
      mutex_(),
      cond_(),
      CurrentBuffer_(new Buffer),
      NextBuffer_(new Buffer),
      buffers_()
{
    CurrentBuffer_->bzero();
    NextBuffer_->bzero();
    buffers_.reserve(16);
}

AsyncLogger::~AsyncLogger()
{
    if (running_)
        stop();
}

void AsyncLogger::append(const char *msg, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (CurrentBuffer_->FreeSize() > len)
    {
        CurrentBuffer_->append(msg, len);
    }
    else
    {
        buffers_.emplace_back(std::move(CurrentBuffer_));
        if (NextBuffer_)
            CurrentBuffer_ = std::move(NextBuffer_);
        else
            CurrentBuffer_.reset(new Buffer);
        CurrentBuffer_->append(msg, len);
        cond_.notify_one();
    }
}

void AsyncLogger::start()
{
    running_ = true;
    thread_.start();
    latch_.wait();
}

void AsyncLogger::stop()
{
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogger::StartLog()
{
    latch_.count_down();
    LogFile output(basename_, RollSize_);
    BufferPtr buffer1(new Buffer);
    BufferPtr buffer2(new Buffer);
    buffer1->bzero();
    buffer2->bzero();
    BufferVector BuffersWrite;
    BuffersWrite.reserve(16);

    while (running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty())
            {
                cond_.wait_for(lock, std::chrono::seconds(Flushinterval_));
            }
            buffers_.emplace_back(std::move(CurrentBuffer_));
            CurrentBuffer_ = std::move(buffer1);
            BuffersWrite.swap(buffers_);
            if (!NextBuffer_)
                NextBuffer_ = std::move(buffer2);
        }

        if (BuffersWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toString().c_str(),
                     BuffersWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            BuffersWrite.erase(BuffersWrite.begin() + 2, BuffersWrite.end());
        }

        for (const auto &buffer : BuffersWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        if (BuffersWrite.size() > 2)
            BuffersWrite.resize(2);

        if (!buffer1)
        {
            buffer1 = std::move(BuffersWrite.back());
            BuffersWrite.pop_back();
            buffer1->reset();
        }

        if (!buffer2)
        {
            buffer2 = std::move(BuffersWrite.back());
            BuffersWrite.pop_back();
            buffer2->reset();
        }

        BuffersWrite.clear();
        output.flush();
    }
    output.flush();
}