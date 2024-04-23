#include "net/Mysql/Mysql.h"
#include "net/Mysql/MysqlPool.h"
#include "base/Logger.h"
#include <format>

using namespace std;
using namespace sql;

Mysql::Mysql(const string &url, const string &usr, const string &password, const string &database, int size)
    : pool_(new MysqlPool(url, usr, password, database, size))
{
}

Mysql::Mysql(const std::string &url, const std::string &usr, const std::string &password, int size)
    : pool_(new MysqlPool(url, usr, password, size))
{
}

Mysql::Mysql(const string &host, const unsigned int port, const string &usr, const string &password, const string &database, int size)
    : pool_(new MysqlPool(host, port, usr, password, database, size))
{
}

Mysql::result Mysql::query(const string &query)
{
    auto con = pool_->GetConnection();
    if (!con)
        return nullptr;
    result res(con->createStatement()->executeQuery(query));
    pool_->ReturnConnection(con);
    return res;
}

int Mysql::update(const string &statement)
{
    auto con = pool_->GetConnection();
    if (!con)
        return 0;
    try
    {
        int n = con->createStatement()->executeUpdate(statement);
        pool_->ReturnConnection(con);
        return n;
    }
    catch (SQLException &e)
    {
        pool_->ReturnConnection(con);
        LOG_ERROR << format("SQLExecption: {} code: {} state: {}", e.what(), e.getErrorCode(), e.getSQLState());
        return 0;
    }
}

bool Mysql::exec(const string &statement)
{
    auto con = pool_->GetConnection();
    if (!con)
        return false;
    try
    {
        con->createStatement()->execute(statement);
        pool_->ReturnConnection(con);
        return true;
    }
    catch (SQLException &e)
    {
        pool_->ReturnConnection(con);
        LOG_ERROR << format("SQLExecption: {} code: {} state: {}", e.what(), e.getErrorCode(), e.getSQLState());
        return false;
    }
}

Mysql::~Mysql()
{
    if (pool_)
        pool_->close();
}