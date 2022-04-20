#include <thread>
#include <iostream>
#include <unistd.h>
#include "tcpserver.h"
#include "config.h"
#include "log.h"
int main()
{
    LoadConf::instance().readConf("server.conf"); // 读取配置文件
    TcpServer* pServer = new TcpServer;
    pServer->onWork();
    pServer->waitServer();
    delete pServer;
}