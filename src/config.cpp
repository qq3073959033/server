#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include "config.h"
#include "log.h"

LoadConf *LoadConf::instance_ = nullptr;
std::mutex LoadConf::mtx_;

LoadConf::LoadConf()
{
}

LoadConf& LoadConf::instance()
{
    if(instance_ == nullptr)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if(instance_ == nullptr)
        {
            instance_ = new LoadConf;
        }
    }
    return *instance_;
}

LoadConf::~LoadConf()
{
}

// 装载配置文件
bool LoadConf::readConf(const char *pconfName)
{
    FILE *fp;
    fp = fopen(pconfName, "r");
    if (fp == NULL)
    {
        return false;
    }
    char linebuf[628] = {};

    // 走到这里，文件打开成功
    while (!feof(fp))
    {
        if (fgets(linebuf, 500, fp) == NULL)
            continue;
        if (linebuf[0] == 0)
            continue;
        // 处理注释行
        if (*linebuf == ';' || *linebuf == ' ' || *linebuf == '#' || *linebuf == '\t' || *linebuf == '\n')
            continue;
lblprocstring:
        // 屁股后边若有换行，回车，空格等都截取掉
        if (strlen(linebuf) > 0)
        {
            if (linebuf[strlen(linebuf) - 1] == 10 || linebuf[strlen(linebuf) - 1] == 13 || linebuf[strlen(linebuf) - 1] == 32)
            {
                linebuf[strlen(linebuf) - 1] = 0;
                goto lblprocstring;
            }
        }
        if (linebuf[0] == 0)
            continue;
        if (*linebuf == '[') // [开头的也不处理,[]用来分组
            continue;
        char *ptmp = strchr(linebuf, '=');
        if (ptmp != NULL)
        {
            Conf* pConf = new Conf;
            memset(pConf, 0, sizeof(Conf));
            strncpy(pConf->confItem, linebuf, (int)(ptmp - linebuf));
            strcpy(pConf->confContent, ptmp + 1);
            subRightStr(pConf->confItem);
            subLeftStr(pConf->confItem);
            subRightStr(pConf->confContent);
            subLeftStr(pConf->confContent);
            confMap_.insert({ pConf->confItem ,pConf->confContent });
            delete pConf;
        }
    }
    fclose(fp);
    return true;
}

void LoadConf::subRightStr(char *string)
{
    size_t len = 0;
    if (string == NULL)
    {
        return;
    }
    len = strlen(string);
    while (len > 0 && string[len - 1] == ' ') //位置换一下
    {
        string[--len] = 0;
    }
    return;
}

void LoadConf::subLeftStr(char *string)
{
    size_t len = 0;
    len = strlen(string);
    char *p_tmp = string;
    if ((*p_tmp) != ' ') // 不是以空格开头
    {
        return;
    }
    // 找第一个不为空格的
    while ((*p_tmp) != '\0')
    {
        if ((*p_tmp) == ' ')
        {
            p_tmp++;
        }   
        else
        {
            break;
        }  
    }
    if ((*p_tmp) == '\0') //全是空格
    {
        *string = '\0';
        return;
    }
    char *p_tmp2 = string;
    while ((*p_tmp) != '\0')
    {
        (*p_tmp2) = (*p_tmp);
        p_tmp++;
        p_tmp2++;
    }
    (*p_tmp2) = '\0';
    return;
}

// 配置文件可能是字符串，也可能是数字
// 根据不同的类型进行转换
const std::string LoadConf::getConfString(const std::string& confItem)
{
    auto iter = confMap_.find(confItem);
    if (iter != confMap_.end())
    {
        return iter->second;
    }
    return "";
}

int LoadConf::getConfInt(const std::string& confItem, const int def)
{
    auto iter = confMap_.find(confItem);
    if (iter != confMap_.end())
    {
        return atoi(iter->second.c_str());
    }
    return def;
}
