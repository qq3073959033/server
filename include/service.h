#ifndef SERVICE_H
#define SERVICE_H
#include <functional>
#include <unordered_map>
using namespace std::placeholders;
class Buffer;
// 业务层
class Service
{
private:
    Service()
    {
        funcMap_.insert({1,std::bind(&Service::Login,this,_1)});
    }
public:
    void Login(Buffer* buffer)
    {
        // 去处理业务
    }
    static Service& instance()
    {
        static Service service;
        return service;
    }
private:
    using logicFunc = void(*)(void);
    // 不同数据类型的包对应不同的处理函数
    std::unordered_map<int,std::function<void(Buffer*)>> funcMap_;
};


#endif