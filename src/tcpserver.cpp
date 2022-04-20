#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "log.h"
#include "tcpserver.h"
#include "config.h"
#include "client.h"
#include "workserver.h"
#include "log.h"

TcpServer::TcpServer()
{
    LoadConf &conf = LoadConf::instance();
    port_ = conf.getConfInt("server_port", 6666);
    ip_ = conf.getConfString("server_ip");
    listenCount_ = conf.getConfInt("listen_count", 10240);
    isDaemon_ = (conf.getConfString("daemon") == "yes" ? true : false);
    workerCount_ = conf.getConfInt("thread_count",4);
}

TcpServer::~TcpServer()
{
    // 关闭套接字
}

void TcpServer::setSockOpt() // 设置端口复用
{
    int n = 1;
    int ret = setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));
    if (ret == -1)
    {
        log_error("setsockopt调用失败");
        exit(-1);
    }
}

void TcpServer::waitServer()
{
    th_.join();
}

void TcpServer::onWork()    // 把服务器的基础流程，例如setsockopt,bind,listen等封装起来，方便外部使用
{
    onDaemon();
    createSock();
    setSockOpt();
    onBind();
    onListen();
    shieldSignal();
    epoll_.createEpoll(ACCEPT_EPOLL);   // 负责accept的线程
    epoll_.epollCtlServer(EPOLL_CTL_ADD,serverFd_,EPOLLIN);
    th_ = std::thread(&TcpServer::onAccept,this);
    // accept线程一切准备就绪，开启worker线程
    runWorker();
}

void TcpServer::runWorker()
{
    workerVec_.resize(workerCount_);
    for (int i = 0; i < workerCount_; ++i)
    {
        workerVec_[i] = new Worker(i);
        workerVec_[i]->startThread();
    }
}

void TcpServer::onDaemon()
{
    if(isDaemon_ == false)
    {
        return;
    }
    pid_t pid = fork();
    if(pid > 0)
    {
        log_info("守护进程的父进程已退出");
        exit(-1);
        // father process
    }
    else if(pid < 0)
    {
        log_error("fork调用失败，创建守护进程失败，程序退出");
        exit(-1);
    }
    else
    {
        if(setsid() == -1)
        {
            log_error("setsid调用失败，创建守护进程失败，程序退出");
            exit(-1);
        }
        umask(0);
        close(0);  // 关闭三个标准文件
        close(1);
        close(2);
        if(chdir("/") == -1)
        {
            log_error("更改守护进程工作路径失败，程序退出");
            exit(-1);
        }
        log_info("守护进程创建成功");
    }
}

// 屏蔽TCP通信中的SIGPIPE信号，这个信号给已关闭的套接字发送数据，会杀死服务器进程
// 调试完成后，把所有的信号都屏蔽，只留下不能屏蔽的19号信号来杀死进程！
void TcpServer::shieldSignal()
{
    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig,13);
    if(sigprocmask(SIG_BLOCK,&sig,NULL) == -1)
    {
        log_error("处理SIGPIPE信号失败，程序退出");
        exit(-1);
    }
}

void TcpServer::onAccept()
{
    while(true)
    {
        socklen_t len = sizeof(sockaddr_in);
        int ret = epoll_.acceptWait();
        if(ret == -1)
        {
            log_error("epoll_wait函数调用失败");
            continue;
        }
        sockaddr_in addr;
        for(int i = 0; i < ret; ++i)
        {
            int clientFd = accept(serverFd_,(sockaddr*)&addr,&len);
            if(clientFd == -1)
            {
                log_error("accept调用失败");
                continue;
            }
            addClient2Woker(clientFd,addr);
        }
    }
}

void TcpServer::addClient2Woker(const int fd,const sockaddr_in& addr)
{
    Worker* minWorker = workerVec_[0];
    int min = workerVec_[0]->getClientCount();
    for(int i = 1; i < workerVec_.size(); ++i)
    {
        int val = workerVec_[i]->getClientCount();
        if(val < min)
        {
            min = val;
            minWorker = workerVec_[i];
        }
    }
    auto p = new Client(fd,addr);
    minWorker->addClient(p);
}

void TcpServer::createSock()
{
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd_ == -1)
    {
        log_error("初始化服务器失败，程序退出");
        exit(-1);
    }
    log_error("服务器初始化成功");
}

void TcpServer::onBind()
{
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port_);
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverFd_, (sockaddr *)&serverAddr_, sizeof(serverAddr_)) == -1)
    {
        log_error("服务器绑定端口失败，程序退出");
        exit(-1);
    }
    log_info("服务器绑定端口成功");
}

void TcpServer::onListen()
{
    if (listen(serverFd_, listenCount_) == -1)
    {
        log_error("服务器监听端口失败，程序退出");
        exit(-1);
    }
    log_info("服务器开启端口监听");
}