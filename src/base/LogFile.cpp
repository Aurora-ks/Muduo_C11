#include "LogFile.h"
#include <filesystem>

LogFile::LogFile(const std::string &name, int roll, int flush, int check)
    : basename_(name),
      RollSize_(roll),
      FlushInterval_(flush),
      CheckEveryN_(check),
      count_(0),
      mutex_(),
      start_(0),
      LastRoll_(0),
      LastFlush_(0),
      file_(),
      filename_()
{
    RollFile();
}

void LogFile::flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    file_.flush();
}

void LogFile::append(const char *msg, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    file_.write(msg, len);
    int size = std::filesystem::file_size(std::filesystem::path(filename_));
    if(size > RollSize_) RollFile();
    else
    {
        ++count_;
        if(count_ >= CheckEveryN_)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t cur = now / kRollPerSeconds_ * kRollPerSeconds_;
            if(cur != start_) RollFile();
            else if(now - LastFlush_ > FlushInterval_)
            {
                LastFlush_ = now;
                file_.flush();
            }
        }
    }
}

bool LogFile::RollFile()
{
    time_t now = 0;
    std::string filename = LogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > LastRoll_)
    {
        filename_ = filename;
        LastRoll_ = now;
        LastFlush_ = now;
        start_ = start;

        file_.close();
        file_.open(filename, std::ios::out | std::ios::app);
        return true;
    }
    return false;
}

std::string LogFile::LogFileName(const std::string &name, time_t *now)
{
    std::string filename;
    filename.reserve(name.size() + 64);
    filename = name;

    char buf[32]{0};
    struct tm *tm;
    *now = ::time(NULL);
    tm = ::localtime(now);
    strftime(buf, sizeof(buf), "-%Y-%m-%d %H:%M:%S", tm);
    filename += buf;
    filename += ".log";

    return filename;
}