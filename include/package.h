#ifndef PACKGE_H
#define PACKGE_H
#include <string>
enum PACKGE_TYPE
{
    LOGIN,
    REGISTER
};


struct PackHead
{
    PackHead(unsigned len,int c)
        : packLen(len)
        , cmd(c)
    { }
    unsigned packLen; // 一个完整的包的长度
    int cmd;        // 数据包的类型
};


// 因为我只想实现服务器中的高性能IO模块，所以没有具体的业务
// 如果要做业务，数据包肯定要用Json或protbuf来写！
// 数据包用结构体仅仅是做测试
struct LoginPack : PackHead
{
    LoginPack(PACKGE_TYPE type)
        : PackHead(sizeof(LoginPack),type)
    {  }
};


#endif