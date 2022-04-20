#ifndef SENDBUFFER_H
#define SENDBUFFER_H
#include <atomic>
#include "package.h"
class Buffer
{
public:
    Buffer(int fd, bool isRecvBuf = true);
    ~Buffer();
    int recvData();
    bool isCompletePack();
    // 交给业务层去解析包
    char *buf() { return buf_; }
    bool isSend() { return lastPos_ > 0; }
private:
    char *buf_;
    int lastPos_;
    int bufLen_;
    int freeLen_;
    int fd_;
};
#endif