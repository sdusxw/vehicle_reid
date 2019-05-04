//
//  simplesvr.cc
//
//  Copyright (c) 2013 Yuji Hirose. All rights reserved.
//  The Boost Software License 1.0
//

#include <cstdio>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <time.h>
#include <boost/thread.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#include "lpr_alg.h"
#include "common.h"
#include "httplib.h"
#include "concurrent_queue.h"

using namespace httplib;
using namespace std;

#define MAX_BUFF_LENGTH     (10*1024*1024)  //共享内存大小10M

class HttpInfo
{
public:
    Request req;
    Response res;
};

class FileInfo
{
public:
    char * p;
    int len;
    string filename;
    string lpr_res_json;
};

ServerConf server_conf;
PVPR pvpr;

concurrent_queue<HttpInfo> g_http_buff;

boost::thread thread_http_handler;

pthread_mutex_t mutex_;

void task_http_handler();

concurrent_queue<FileInfo> g_file_queue;

boost::thread thread_save_file;

void task_save_file();

string dump_headers(const Headers &headers) {
    string s;
    char buf[BUFSIZ];
    
    for (const auto &x : headers) {
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }
    
    return s;
}

void save_file(const char * file_content, int len, string file_name, string lpr_res_json)
{
    string path="",file="";
    if (split_filename(file_name, path, file)) {
        path = server_conf.image_base + "/" + path;
        create_dir(path.c_str());
    }
    file_name = server_conf.image_base + "/" + file_name;
    ofstream output( file_name, ios::out | ios::binary );
    if( ! output )
    {
        cerr << "Open output file error!" << endl;
        if (file_content) {
            free((void*)file_content);
            file_content = NULL;
        }
        return;
    }
    
    output.write (file_content, len);
    
    output.close();
    if (file_content) {
        free((void*)file_content);
        file_content = NULL;
    }
    //保存识别结果json文件
    string json_name = file_name + ".json";
    ofstream json_output( json_name, ios::out | ios::binary );
    if( ! json_output )
    {
        cerr << "Open json_output file error!" << endl;
        return;
    }
    
    json_output.write (lpr_res_json.c_str(), lpr_res_json.length());
    
    json_output.close();
}

string dump_multipart_files(const MultipartFiles &files) {
    string s;
    char buf[BUFSIZ];
    
    s += "--------------------------------\n";
    
    for (const auto &x : files) {
        const auto &name = x.first;
        const auto &file = x.second;
        
        snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
        s += buf;
        
        snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
        s += buf;
        
        snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
        s += buf;
        
        snprintf(buf, sizeof(buf), "text offset: %lu\n", file.offset);
        s += buf;
        
        snprintf(buf, sizeof(buf), "text length: %lu\n", file.length);
        s += buf;
        
        s += "----------------\n";
    }
    
    return s;
}

string log(const Request &req, const Response &res) {
    string s;
    char buf[BUFSIZ];
    
    s += "================================\n";
    
    snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
             req.version.c_str(), req.path.c_str());
    s += buf;
    
    string query;
    for (auto it = req.params.begin(); it != req.params.end(); ++it) {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%c%s=%s",
                 (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
                 x.second.c_str());
        query += buf;
    }
    snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    s += buf;
    
    s += dump_headers(req.headers);
    s += dump_multipart_files(req.files);
    
    s += "--------------------------------\n";
    
    snprintf(buf, sizeof(buf), "%d\n", res.status);
    s += buf;
    s += dump_headers(res.headers);
    
    return s;
}

void task_http_handler()
{
    while (true) {
        HttpInfo http_info;
        g_http_buff.wait_and_pop(http_info);
        auto body = "{\"code\":0}";
        http_info.res.set_content(body, "application/json");
    }
}

void task_save_file()
{
    while (true) {
        FileInfo file_info;
        g_file_queue.wait_and_pop(file_info);
        save_file((const char*)file_info.p, file_info.len, file_info.filename, file_info.lpr_res_json);
    }
}

int main(int argc, const char **argv) {
    //算法库初始化
    if(!vlpr_init())
    {
        cout<<"LPR_ALG init Fail!"<<endl;
        exit(-1);
    }else{
        cout<<"LPR_ALG init OK!"<<endl;
    }
    
    pvpr=(PVPR)malloc(sizeof(VPR));
    
    pthread_mutex_init(&mutex_,NULL);
    
    //读取服务器配置文件
    server_conf.server_ip="0.0.0.0";
    server_conf.server_port=80;
    server_conf.image_base="/data";
    if(read_config(server_conf))
    {
        cout << "Read Server Configure OK!" << endl;
    }else{
        cout << "Read Server Configure Fail!" << endl;
    }
    Server svr;
    svr.Post("/chpAnalyze", [](const Request &req, Response &res) {
        Json::Value json_res;
        Json::StreamWriterBuilder writerBuilder;
        std::ostringstream os;
        bool have_jpg = false;
        std::string file_name;
        std::string file_content;
        int file_len;
        for (const auto &x : req.files) {
            const auto &name = x.first;
            const auto &file = x.second;
            if(name != "file") continue;
            if(have_jpg) cout << "more than one pic per post" << endl;
            file_content = req.body.substr(file.offset, file.length);
            file_name = file.filename;
            file_len = (int)file.length;
            have_jpg = true;
        }
        cout << get_time_ms() << "\t";
        cout << file_name << "\t";
        cout << file_len << "\t";
        std::string body;
        if(have_jpg)
        {
            clock_t t=clock();
            pthread_mutex_lock(&mutex_);
            if(vlpr_analyze((const unsigned char *)file_content.c_str(), file_len, pvpr))
            {
                cout << (clock()-t)/1000 << "\t";
                cout << pvpr->license << "\t" << pvpr->color << endl;
                Json::Value json_result;
                json_result["fx_device_id"]="beichuang_01";
                json_result["fx_duration"]=(int)((clock()-t)/1000);
                json_result["vehlic"]=pvpr->license;
                json_result["lic_color"]=pvpr->nColor;
                Json::Value json_rect;
                json_rect["left"]=pvpr->left;
                json_rect["right"]=pvpr->right;
                json_rect["top"]=pvpr->top;
                json_rect["bottom"]=pvpr->bottom;
                json_result["rect"]=json_rect;
                Json::Value json_results;
                json_results["code"]=0;
                json_results["message"]="successd";
                json_results["filename"]=file_name;
                json_results["vender_id"]="sjk-beichuang-lpa";
                json_results["result"]=json_result;
                json_res["code"]=0;
                json_res["message"]="successd";
                Json::Value array;
                array.append(json_results);
                json_res["results"]=array;
            }else{
                cout << (clock()-t)/1000 << "\t";
                cout << "Fail" << endl;
                
                Json::Value json_results;
                json_results["code"]=2;
                json_results["message"]="NO_PLATE";
                json_results["filename"]=file_name;
                json_results["vender_id"]="sjk-beichuang-lpa";
                
                json_res["code"]=0;
                json_res["message"]="successd";
                Json::Value array;
                array.append(json_results);
                json_res["results"]=array;
                file_name += ".x.jpg";
            }
            pthread_mutex_unlock(&mutex_);
        }else{
            Json::Value json_results;
            json_results["code"]=2;
            json_results["message"]="NO_PLATE";
            json_results["filename"]=file_name;
            json_results["vender_id"]="sjk-beichuang-lpa";
            
            json_res["code"]=0;
            json_res["message"]="successd";
            Json::Value array;
            array.append(json_results);
            json_res["results"]=array;
            file_name += ".x.jpg";
        }
        std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
        jsonWriter->write(json_res, &os);
        body = os.str();
        res.set_content(body, "application/json");
        FileInfo file_info;
        file_info.len = file_len;
        file_info.p = (char*)malloc(file_len);
        memcpy(file_info.p, (const unsigned char *)file_content.c_str(), file_len);
        file_info.filename = file_name;
        file_info.lpr_res_json = body;
        g_file_queue.push(file_info);
    });
    
    thread_save_file = boost::thread(boost::bind(&task_save_file));
  
    cout << "The server started at port " << server_conf.server_port << "..." << endl;
    
    svr.listen(server_conf.server_ip.c_str(), server_conf.server_port);
    
    return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
