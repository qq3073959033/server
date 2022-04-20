#include <sys/types.h>
#include <sys/socket.h>
#include "buffer.h"
#include "config.h"
#include "log.h"
Buffer::Buffer(int fd, bool isRecvBuf)
    : lastPos_(0)
    , fd_(fd)
{
    LoadConf &conf = LoadConf::instance();
    if (isRecvBuf)
    {
        bufLen_ = conf.getConfInt("recv_buffer_size", 1024);
    }
    else
    {
        bufLen_ = conf.getConfInt("send_buffer_size", 1024);
    }
    buf_ = new char[bufLen_];
    freeLen_ = bufLen_; // 缓冲区剩余的空间
}

Buffer::~Buffer()
{
    delete buf_;
    buf_ = nullptr;
}

int Buffer::recvData()
{
    if (freeLen_ == 0)
    {
        // 可能是现在业务太繁忙了，造成接收缓冲区堵塞
        // 不用关闭套接字，等待服务器处理完数据再去接收
        // 根据业务设置自己的缓冲区大小
        log_debug("接收缓冲区满了");
        return -2; // -2 表示接收缓冲区满了
    }
    int ret = recv(fd_, buf_ + lastPos_, freeLen_, 0);
    if (ret == -1)
    {
        log_error("recv调用失败");
        return -1;
    }
    // 接收数据完成
    lastPos_ += ret;
    freeLen_ -= ret;
    return ret;
}

bool Buffer::isCompletePack()
{
    // 包头是8个字节
    if (lastPos_ < 8)
    {
        return false;
    }
    PackHead *head = (PackHead *)buf_;
    int len = head->packLen;
    if (lastPos_ >= len) // 有一个完整的包
    {
        return true;
    }
    return false;
}
