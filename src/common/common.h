#ifndef _LPR_COMMON_H
#define _LPR_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <vector>
#include <time.h>

#define CONFIGURE_FILE    "../conf/server.conf"

class ServerConf
{
public:
    std::string server_ip;
    int     server_port;
    std::string image_base;
};

//全局日志文件变量
extern std::fstream g_log_file;
//控制台消息输出：输出格式为时间+消息
void msg_print(std::string msg);
//日志输出初始化，生成日志文件，追加模式
void log_init();
//日志输出：输出格式为时间+日志消息
void log_output(std::string msg, bool std_print=true);
//获取时间，精确到微秒
std::string get_time_us();
//获取时间，精确到毫秒
std::string get_time_ms();
//获取时间，精确到妙
std::string get_time_sec();
//获取日期，精确到天
std::string get_time_date();
//获取Unix时间戳
long get_unix_ts();
//获取ip地址
std::string get_local_ip();
//获取ip地址，需要输入网卡设备名称
bool get_local_ip(std::string dev, std::string & str_ip);
//创建文件夹
int create_dir(const char *s_path_name);
//进程互斥
bool is_have_instance();
//string 字符串分割
std::vector<std::string> string_split(std::string str, std::string pattern);
//读取配置文件
bool read_config(ServerConf &conf);
//分割路径和文件
bool split_filename(std::string filename, std::string &path, std::string &file);
//字符编码转换
/*
bool gbk2utf8(std::string gbk, std::string &utf8);
bool utf8togb2312(std::string utf8, std::string &gb2312);
int gb2312toutf8(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int utf8togb2312(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int code_convert(char *from_charset, char *to_charset, char *inbuf,
                 size_t inlen, char *outbuf, size_t outlen);
*/
#endif
