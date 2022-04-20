#ifndef WORKSERVER_H
#define WORKSERVER_H
#include "epoll.h"
#include <thread>
#include <unordered_map>
#include <queue>
#include <atomic>
#include <mutex>

class Client;

class Worker
{
public:
    Worker(int id);
    void workThread();
    void addClientCount();
    void subClientCount();
    int getClientCount();
    void addClient(Client *pClient);
    void startThread();
    void shieldSend();
    void sheildData(Client *pClient);
    void Worker::destroyClient(int fd);
private:
    std::queue<Client *> clientBuffer_;
    std::unordered_map<int, Client *> clientMap_; // socket对应一个Client客户端对象
    std::atomic_int clientCount_;                 // 缓冲区中的客户端数量
    Epoll epoll_;
    int threadId_;
    std::thread th_;
    std::mutex queMtx_;
};

#endif