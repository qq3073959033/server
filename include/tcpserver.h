#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <queue>
#include <vector>
#include "epoll.h"

class Epoll;
class Worker;

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();
    void onWork(); // 把服务器的基础流程，例如setsockopt,bind,listen等封装起来，方便外部使用
    void waitServer();

private:
    void createSock(); // 创建服务端套接字
    void onBind();     // 绑定服务器端口
    void onListen();   // 开启监听队列
    void setSockOpt(); // 设置端口复用
    void onAccept();
    void shieldSignal(); // 屏蔽TCP通信中的SIGPIPE信号，这个信号给已关闭的套接字发送数据，会杀死服务器进程
                         // 调试完成后，把所有的信号都屏蔽，只留下不能屏蔽的19号信号来杀死进程！
    void onDaemon();                                             // 根据配置文件，决定是不是要用守护进程的方式启动
    void runWorker();                                            // 开启worker线程处理网络IO
    void addClient2Woker(const int fd, const sockaddr_in &addr); // accept完成的客户端,加入到worker线程中
private:
    short port_;     // 配置文件读取端口
    std::string ip_; // 配置文件读取服务器IP地址
    int serverFd_;
    int listenCount_; // 配置文件读取监听队列长度
    sockaddr_in serverAddr_;
    Epoll epoll_;                     // 负责accept的epoll对象
    std::thread th_;                  // 负责accept的thread对象
    bool isDaemon_;                   // 是否按照守护进程的方式启动
    int workerCount_;                 // 根据配置文件读取worker线程的数量
    std::vector<Worker *> workerVec_; // 存储网络IO的工作线程对象
};

#endif