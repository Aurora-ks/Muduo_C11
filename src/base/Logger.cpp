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

void DefaultOutput(const char *msg, int len)
{
    fwrite(msg, len, 1, stdout);
}

void DefaultFlush()
{
    fflush(stdout);
}

Logger::OutputFun g_output = DefaultOutput;
Logger::FlushFun g_flush = DefaultFlush;

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
    stream_ << Timestamp::now().toString() << ' ' << LogLevelName[level_];
}

Logger::Logger(std::string file, int line, LogLevel level)
    : level_(level),
      line_(line),
      basename_(file)
{
    stream_ << Timestamp::now().toString() << ' ' << LogLevelName[level_];
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

void Logger::SetLogLevel(Logger::LogLevel level)
{
    g_LogLevel = level;
}

void Logger::SetOutput(OutputFun f)
{
    g_output = f;
}
void Logger::SetFlush(FlushFun f)
{
    g_flush = f;
}