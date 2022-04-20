#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
class Client;
enum EPOLL_TYPE
{
    IO_EPOLL,
    ACCEPT_EPOLL
};

class Epoll
{
public:
    Epoll();
    ~Epoll();
    void createEpoll(EPOLL_TYPE epollType);
    int epollCtlServer(int op, int sock, unsigned events);
    int epollCtlClient(int op, Client *pClient, unsigned events);
    int iOWait();
    int acceptWait();
    void destroy(); // 回收epoll资源
    epoll_event *event() { return pEvent_; }
private:
    int epollFd_;
    int IOCount_; // 一次可以监听io事件
    int acceptCount_;
    epoll_event *pEvent_;
};

#endif