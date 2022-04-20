#ifndef CONFIGURE_H
#define CONFIGURE_H
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
struct Conf
{
	char confItem[128];
	char confContent[500]; // 保存配置内容
};

class LoadConf
{
public:
	~LoadConf();
	bool readConf(const char *pconfName);
	static LoadConf &instance();
	int getConfInt(const std::string &confItem, const int def);
	const std::string getConfString(const std::string &confItem);
private:
	LoadConf();
	void subRightStr(char *string);
	void subLeftStr(char *string);
private:
	std::unordered_map<std::string, std::string> confMap_;
	static LoadConf *instance_; // 配置文件内容也许比较大，就不用懒汉单例了，直接放堆上吧
	static std::mutex mtx_;		// 防止构造多个单例对象，采用双重检查加锁的方式
};

#endif