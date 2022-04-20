#ifndef CLIENT_H
#define CLIENT_H
#include <netinet/in.h>
#include <chrono>
#include "buffer.h"
#include "objectpool.h"
// 封装客户端类，用来做心跳机制,收发包机制
// 调用业务层的接口处理解析完成的数据包
class Client : public ObjectPoolBase<Client,2000>
{
public:
    Client(int fd, const sockaddr_in &addr);
    int fd() { return clientFd_; }
    int readData() { return recvBuf_.recvData(); }
    bool isCompletePack() { return recvBuf_.isCompletePack(); }
    char* buf() { return recvBuf_.buf(); }
    bool isSend() { return sendBuf_.isSend(); }
    Buffer* recvBuffer() { return &recvBuf_; }
    Client(const Client &) = delete;
    Client(Client &&) = delete;
    Client &operator=(const Client &) = delete;
    Client &operator=(Client &&) = delete;
private:
    int clientFd_;
    Buffer recvBuf_;
    Buffer sendBuf_;
    sockaddr_in addr_;
};

#endif