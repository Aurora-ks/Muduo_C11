#pragma once

#include "base/nocopyable.h"
#include "base/ThreadSafeQueue.ipp"
#include <mysql-cppconn-8/mysql/jdbc.h>
#include <string>
#include <atomic>

class MysqlPool : nocopyable
{
public:
    MysqlPool(const std::string &url, const std::string &usr, const std::string &password, const std::string &database, int size);
    MysqlPool(const std::string &url, const std::string &usr, const std::string &password, int size);
    MysqlPool(const std::string &host, const unsigned int port, const std::string &usr, const std::string &password, const std::string &database, int size);
    ~MysqlPool();

    std::unique_ptr<sql::Connection> GetConnection();
    void ReturnConnection(std::unique_ptr<sql::Connection> &con);
    void close();
    int size() const { return size_; }
private:
    int size_;
    ThreadSafeQueue<std::unique_ptr<sql::Connection>> pool_;
    std::atomic_bool runing_;
};