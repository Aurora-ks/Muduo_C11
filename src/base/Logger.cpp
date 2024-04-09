#include "Logger.h"

void Logger::log(std::string msg)
{
    switch (LogLevel_)
    {
    case INFO:
        std::cout << "[info] ";
        break;
    case ERROR:
        std::cout << "[error] ";
        break;
    case FATAL:
        std::cout << "[fatal] ";
        break;
    case DEBUG:
        std::cout << "[debug] ";
    }
    std::cout << Timestamp::now().toString() << ":";
    std::cout << msg << std::endl;
}

void Logger::log(int level, std::string msg)
{
    SetLevel(level);
    log(msg);
}