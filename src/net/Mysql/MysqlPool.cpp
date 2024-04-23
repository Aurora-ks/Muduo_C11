#include "net/Mysql/MysqlPool.h"
#include "base/Logger.h"

using namespace std;
using namespace sql;

MysqlPool::MysqlPool(const string &host, const unsigned int port, const string &usr, const string &password, const string &database, int size)
    : size_(size),
      runing_(true)
{
    string url = "tcp:://";
    url = url + host + ":" + to_string(port);
    for (int i = 0; i < size; ++i)
    {
        Driver *drive = get_driver_instance();
        unique_ptr<Connection> con(drive->connect(url, usr, password));
        if (!con->isValid())
            LOG_FATAL << "MysqlPool Creat Failed";
        con->setSchema(database);
        pool_.push(move(con));
    }
}

MysqlPool::MysqlPool(const string &url, const string &usr, const string &password, const string &database, int size)
    : size_(size),
      runing_(true)
{
    for (int i = 0; i < size; ++i)
    {
        Driver *drive = get_driver_instance();
        unique_ptr<Connection> con(drive->connect(url, usr, password));
        if (!con->isValid())
            LOG_FATAL << "MysqlPool Creat Failed";
        con->setSchema(database);
        pool_.push(move(con));
    }
}

MysqlPool::MysqlPool(const string &url, const string &usr, const string &password, int size)
    : size_(size),
      runing_(true)
{
    for (int i = 0; i < size; ++i)
    {
        Driver *drive = get_driver_instance();
        unique_ptr<Connection> con(drive->connect(url, usr, password));
        if (!con->isValid())
            LOG_FATAL << "MysqlPool Creat Failed";
        pool_.push(move(con));
    }
}

MysqlPool::~MysqlPool()
{
    while (!pool_.empty())
    {
        pool_.WaitPop();
    }
}

unique_ptr<sql::Connection> MysqlPool::GetConnection()
{
    if (!runing_)
        return nullptr;
    auto ConPtr = pool_.WaitPop();
    auto res = move(*ConPtr.get());
    ConPtr.reset();
    return res;
}

void MysqlPool::ReturnConnection(unique_ptr<Connection> &con)
{
    if (!runing_)
        return;
    pool_.push(move(con));
}

void MysqlPool::close()
{
    runing_ = false;
    pool_.NotifyAll();
}