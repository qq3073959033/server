#include <unistd.h>
#include "client.h"
#include "workserver.h"
#include "client.h"
#include "log.h"
#include "service.h"
#include <iostream>
using namespace std;
Worker::Worker(int id)
    : threadId_(id), clientCount_(0)
{
    epoll_.createEpoll(IO_EPOLL);
}

// 前期测试用
void pack(char *buf)
{
    PackHead *head = (PackHead *)buf;
    cout << head->cmd << " " << head->packLen << endl;
}

void Worker::workThread()
{
    while (true)
    {
        queMtx_.lock();
        while (!clientBuffer_.empty())
        {
            // 直接构造，减少拷贝开销
            clientMap_.emplace(clientBuffer_.front()->fd(), clientBuffer_.front());
            clientBuffer_.pop();
        }
        queMtx_.unlock();
        // 没有套接字需要处理读写事件，继续去检查缓存队列
        if (clientMap_.empty())
        {
            continue;
        }
        shieldSend(); // 检查发送缓冲区是否有数据，有数据才添加写事件！
                      // 内核发送缓冲区还有剩余空间，epoll_wait都会认为这个套接字是可写的！！
        int ret = epoll_.iOWait();
        epoll_event *p = epoll_.event();
        for (int i = 0; i < ret; ++i)
        {
            sheildData((Client *)p[i].data.ptr);
        }
    }
}

void Worker::destroyClient(int fd)
{
    auto iter = clientMap_.find(fd);
    if(iter == clientMap_.end())
    {
        // 不应该出现，调试用，先打个日志
        log_debug("找不到要删除的客户端");
        return;
    }
    delete iter->second;
    clientMap_.erase(iter);
}

void Worker::sheildData(Client *pClient)
{
    if (pClient != nullptr)
    {
        int ret = pClient->readData();
        if (ret == 0)
        {
            // 客户端断线了！！！
            destroyClient(pClient->fd());
        }
        else
        {
            // 接收完成，处理业务！检查是不是有一个完整的数据包！
            // 如果有完整的数据包，就去处理业务吧！
            if (pClient->isCompletePack())
            {
                // 有完整的包了！！上传给业务层进行处理！
                // 业务层：不知道想做什么，以后再来实现吧！
            }
        }
    }
    else
    {
        // 这里不应该出现为空的情况，因为在accept之后就完成了注册
        // 为了DEBUG，先加上！
        log_error("epoll.ptr为空，注册出错");
    }
}

void Worker::shieldSend()
{
    // 接收缓冲区有数据，才添加写事件！
    for (auto var : clientMap_)
    {
        if (var.second->isSend())
        {
            epoll_.epollCtlClient(EPOLL_CTL_ADD, var.second, EPOLLOUT);
        }
    }
}

void Worker::addClientCount()
{
    clientCount_++;
}

void Worker::startThread()
{
    th_ = std::thread(&Worker::workThread, this);
    th_.detach();
}

void Worker::subClientCount()
{
    clientCount_--;
}

int Worker::getClientCount()
{
    return clientBuffer_.size() + clientMap_.size();
}

void Worker::addClient(Client *pClient)
{
    // 如果添加读事件都失败了，直接关闭这个套接字
    // 这里可以给用户先返回错误信息
    if (epoll_.epollCtlClient(EPOLL_CTL_ADD, pClient, EPOLLIN) == -1)
    {
        logic_error("epoll_ctl调用失败,关闭客户端套接字");
        close(pClient->fd());
        return;
    }
    // 添加读事件成功了，套接字进入worker队列中，处理读事件
    std::lock_guard<std::mutex> lock(queMtx_);
    clientBuffer_.push(pClient);
}
