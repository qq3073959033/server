#include "client.h"

// 把客户端的sockaddr_in信息保存起来
// 以后如果要做黑名单的话，可以用ip地址来做限制
Client::Client(int fd,const sockaddr_in& addr)
        : clientFd_(fd)
        , sendBuf_(fd,false)
        , recvBuf_(fd)
        , addr_(addr)
{
        
}