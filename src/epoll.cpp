#include "epoll.h"
#include "config.h"
#include "log.h"
#include "client.h"

Epoll::Epoll()
    : epollFd_(-1)
    , pEvent_(nullptr)
{
    LoadConf &conf = LoadConf::instance();
    IOCount_ = conf.getConfInt("epoll_io_count", 1024);
    acceptCount_ = conf.getConfInt("epoll_accept_count", 5);
}

Epoll::~Epoll()
{
}

void Epoll::destroy() // 预留接口
{
}

void Epoll::createEpoll(EPOLL_TYPE epollType)
{
    if (epollType == IO_EPOLL)
    {
        pEvent_ = new epoll_event[IOCount_];
        epollFd_ = epoll_create(IOCount_);
        if (epollFd_ == -1)
        {
            log_error("create epoll socket false");
            exit(-1);
        }
    }
    else
    {
        pEvent_ = new epoll_event[acceptCount_];
        epollFd_ = epoll_create(acceptCount_);
        if (epollFd_ == -1)
        {
            log_error("create epoll socket false");
            exit(-1);
        }
    }
}

int Epoll::epollCtlServer(int op, int sock, unsigned events)
{
    epoll_event event;
    event.data.fd = sock;
    event.events = events;
    int ret = epoll_ctl(epollFd_, op, sock, &event);
    if (ret == -1)
    {
        log_error("add socket to epoll false");
        return -1;
    }
    return ret;
}

int Epoll::epollCtlClient(int op,Client* pClient,unsigned events)
{
    epoll_event event;
    event.data.fd = pClient->fd();
    event.data.ptr = pClient;   // 将epoll的ptr和客户端对象关联，发生读写事件要操作pClient!!
    event.events = events;
    int ret = epoll_ctl(epollFd_, op, pClient->fd(), &event);
    if (ret == -1)
    {
        log_error("add socket to epoll false");
        return -1;
    }
    return ret;
}

int Epoll::acceptWait() // 等待accept事件
{
    int ret = epoll_wait(epollFd_, pEvent_, acceptCount_, -1);
    if (ret == -1)
    {
        log_error("epoll_wait call false");
        return ret;
    }
    return ret;
}

int Epoll::iOWait()     // 等待网络IO事件
{
    int ret = epoll_wait(epollFd_, pEvent_, IOCount_, -1);
    if (ret == -1)
    {
        log_error("epoll_wait call false");
        return ret;
    }
    return ret;
}