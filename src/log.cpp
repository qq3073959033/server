#include "log.h"
#include "config.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

PrintLog* PrintLog::instance_ = nullptr;
std::mutex PrintLog::instanceMtx_;

PrintLog::PrintLog()
{
    logPath_ = LoadConf::instance().getConfString("log_path");
    fd_ = open(logPath_.c_str(), O_WRONLY | O_TRUNC | O_CREAT);
    if (fd_ == -1)
    {
        exit(-1);
    }
    logThread_ = std::thread([this]()
                             { this->logThreadFunc(); });
    logThread_.detach();
}

PrintLog::~PrintLog()
{
    close(fd_);
}

// 日志打印线程
void PrintLog::logThreadFunc()
{
    while (true)
    {
        std::string str;
        {
            std::unique_lock<std::mutex> lock(queMtx_);
            while(logQue_.empty())
            {
                cond_.wait(lock);
            }
            // 走到这儿，一定是拿到锁的！
            str = logQue_.front();
            logQue_.pop();
            // 释放锁
        }
        write(fd_,str.c_str(),str.length());
    }
}

void PrintLog::log_info(const std::string &str)
{
    std::string s;
    time_t now = time(NULL);
    tm *t = localtime(&now);
    char buf[1024];
    sprintf(buf, "[%s] [%d/%d/%d] %02d:%02d:%02d %s",
            "LOG_INFO",
            t->tm_year + 1900, t->tm_mon + 1,
            t->tm_mday, t->tm_hour,
            t->tm_min, t->tm_sec,
            str.c_str());
    s += buf;
    s += "\n";
    std::lock_guard<std::mutex> lock(queMtx_);
    logQue_.push(s);
    cond_.notify_one();
}

void PrintLog::log_debug(const std::string &str)
{
    std::string s;
    time_t now = time(NULL);
    tm *t = localtime(&now);
    char buf[1024];
    sprintf(buf, "[%s] [%d/%d/%d] %02d:%02d:%02d %s",
            "LOG_DEBUG",
            t->tm_year + 1900, t->tm_mon + 1,
            t->tm_mday, t->tm_hour,
            t->tm_min, t->tm_sec,
            str.c_str());
    s += buf;
    s += "\n";
    std::lock_guard<std::mutex> lock(queMtx_);
    logQue_.push(s);
    cond_.notify_one();
}

void PrintLog::log_error(const std::string &str)
{
    std::string s;
    time_t now = time(NULL);
    tm *t = localtime(&now);
    char buf[1024];
    sprintf(buf, "[%s] [%d/%d/%d] %02d:%02d:%02d %s",
            "LOG_ERROR",
            t->tm_year + 1900, t->tm_mon + 1,
            t->tm_mday, t->tm_hour,
            t->tm_min, t->tm_sec,
            str.c_str());
    s += buf;
    s += "\n";
    std::lock_guard<std::mutex> lock(queMtx_);
    logQue_.push(s);
    cond_.notify_one();
}

PrintLog& PrintLog::instance()
{
    if(instance_ == nullptr)
    {
        std::lock_guard<std::mutex> lock(instanceMtx_);
        if(instance_ == nullptr)
        {
            instance_ = new PrintLog;
        }
    }
    return *instance_;
}
