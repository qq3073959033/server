#ifndef LOG_H
#define LOG_H
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

enum LOG_LEVEL
{
    LOG_INFO,  // 常规信息
    LOG_DEBUG, // 调试信息
    LOG_ERROR  // 错误信息
};

class PrintLog
{
private:
    PrintLog();
public:
    ~PrintLog();

    void logThreadFunc();

    void log_info(const std::string &str);

    void log_debug(const std::string &str);

    void log_error(const std::string &str);

    static PrintLog& instance();
public:
    static PrintLog* instance_;
private:
    std::thread logThread_; // 开一个线程去打印日志，采用生产者消费者线程去打
    std::queue<std::string> logQue_;
    std::mutex queMtx_;
    std::condition_variable cond_;
    static std::mutex instanceMtx_;
    std::string logPath_;
    int fd_;
};

inline void log_info(const std::string& str)
{
    PrintLog::instance().log_info(str);
}

inline void log_error(const std::string& str)
{
    PrintLog::instance().log_error(str);
}

inline void log_debug(const std::string& str)
{
    PrintLog::instance().log_debug(str);
}


#endif
