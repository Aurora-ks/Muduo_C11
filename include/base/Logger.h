#pragma once

#include "LogStream.h"
#include "Timestamp.h"

#define LOG_DEBUG_ON 1
class T;

class Logger
{
public:
    enum LogLevel
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LOG_NUM
    };

    Logger(std::string file, int line);
    Logger(std::string file, int line, LogLevel level);
    ~Logger();

    LogStream& stream() { return stream_; }

    static LogLevel loglevel();
    static void SetLogLevel(LogLevel level);

    using OutputFun = void(*) (const char* msg, int len);
    using FlushFun = void(*) ();

    static void SetOutput(OutputFun f);
    static void SetFlush(FlushFun f);
private:
    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    std::string basename_;
};

extern Logger::LogLevel g_LogLevel;

inline Logger::LogLevel Logger::loglevel()
{
    return g_LogLevel;
}

#define LOG_INFO \
    if(Logger::loglevel() <= Logger::LogLevel::INFO) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::INFO).stream()

#define LOG_DEBUG \
    if(Logger::loglevel() <= Logger::LogLevel::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG).stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::LogLevel::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL).stream()