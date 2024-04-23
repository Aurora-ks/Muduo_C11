#pragma once

#include "base/nocopyable.h"
#include "net/Mysql/MysqlPool.h"
#include <memory>

class Mysql : nocopyable
{
public:
    using result = std::unique_ptr<sql::ResultSet>;

    Mysql(const std::string &url, const std::string &usr, const std::string &password, const std::string &database, int size = 0);
    Mysql(const std::string &url, const std::string &usr, const std::string &password, int size = 0);
    Mysql(const std::string &host, const unsigned int port, const std::string &usr, const std::string &password, const std::string &database, int size = 0);
    ~Mysql();

    result query(const std::string &query);
    int update(const std::string &statement);
    bool exec(const std::string &statement);
private:
    // std::unique_ptr<sql::Connection> con_;
    std::unique_ptr<MysqlPool> pool_;
};