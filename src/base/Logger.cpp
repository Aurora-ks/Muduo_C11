#include "Logger.h"

#if LOG_DEBUG_ON
Logger::LogLevel g_LogLevel = Logger::LogLevel::DEBUG;
#else
Logger::LogLevel g_LogLevel = Logger::LogLevel::INFO;
#endif

const char *LogLevelName[Logger::LogLevel::LOG_NUM] =
    {
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
};

/* class T
{
public:
    T(const char *str, size_t len)
        : str_(str),
          len_(len) {}
    const char *str_;
    size_t len_;
};

inline LogStream& operator << (LogStream& s, T v)
{
    s.append(v.str_, v.len_);
} */

Logger::Logger(std::string file, int line)
    : level_(LogLevel::INFO),
      line_(line),
      basename_(file)
{
    stream_ << Timestamp::now().toString() << LogLevelName[level_];
}

Logger::Logger(std::string file, int line, LogLevel level)
    : level_(level),
      line_(line),
      basename_(file)
{
    stream_ << Timestamp::now().toString() << LogLevelName[level_];
}

Logger::~Logger()
{
    stream_ << " - " << basename_ << ':' << line_ << '\n';
    const LogStream:: Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if(level_ == LogLevel::FATAL)
    {
        g_flush();
        abort();
    }
}

Logger::OutputFun g_output = DefaultOutput;
Logger::FlushFun g_flush = DefaultFlush;

void Logger::SetLogLevel(Logger::LogLevel level)
{
    g_LogLevel = level;
}

Logger::LogLevel Logger::loglevel()
{
    return g_LogLevel;
}

void DefaultOutput(const char *msg, int len)
{
    fwrite(msg, len, 1, stdout);
}

void DefaultFlush()
{
    fflush(stdout);
}

void Logger::SetOutput(OutputFun f)
{
    g_output = f;
}
void Logger::SetFlush(FlushFun f)
{
    g_flush = f;
}

/* void Logger::log(std::string msg)
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
    std::cout << Timestamp::now().toString() << ": ";
    std::cout << msg << std::endl;
}

void Logger::log(int level, std::string msg)
{
    SetLevel(level);
    log(msg);
} */