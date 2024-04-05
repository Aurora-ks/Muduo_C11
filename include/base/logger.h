#pragma once

#include <string>
#include <iostream>
#include "nocopyable.h"
#include "singleton.ipp"
#include "timestamp.h"

enum LogLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

class Logger final : public Singleton<Logger>
{
public:
    void SetLevel(int level){LogLevel_ = level;}
    void log(std::string msg);
    void log(int level, std::string msg);
private:
    int LogLevel_;
};

#define LOG_INFO(MsgFormat, ...) \
    do{ \
        char buf[1024]{0}; \
        snprintf(buf, 1024, MsgFormat, ##__VA_ARGS__); \
        Logger::GetInstance()->log(INFO, buf); \
    }while(0);

#define LOG_WARNING(MsgFormat, ...) \
    do{ \
        char buf[1024]{0}; \
        snprintf(buf, 1024, MsgFormat, ##__VA_ARGS__); \
        Logger::GetInstance()->log(WARNING, buf); \
    }while(0);

#define LOG_ERROR(MsgFormat, ...) \
    do{ \
        char buf[1024]{0}; \
        snprintf(buf, 1024, MsgFormat, ##__VA_ARGS__); \
        Logger::GetInstance()->log(ERROR, buf); \
    }while(0);

#define LOG_FATAL(MsgFormat, ...) \
    do{ \
        char buf[1024]{0}; \
        snprintf(buf, 1024, MsgFormat, ##__VA_ARGS__); \
        Logger::GetInstance()->log(FATAL, buf); \
        exit(-1); \
    }while(0);

#ifdef DEBUG_MOD
#define LOG_DEBUG(MsgFormat, ...) \
    do{ \
        char buf[1024]{0}; \
        sprintf(buf, 1024, MsgFormat, ##__VA_ARGS__); \
        Logger::GetInstance()->log(DEBUGL, buf); \
    }while(0)
#else
    #define LOG_DEBUG(MsgFormat, ...)
#endif