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

concurrent_queue<HttpInfo> g_http_buff;

boost::thread thread_http_handler;

pthread_mutex_t mutex_;

void task_http_handler();

string dump_headers(const Headers &headers) {
    string s;
    char buf[BUFSIZ];
    
    for (const auto &x : headers) {
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }
    
    return s;
}

void save_files(const Request &req, const MultipartFiles &files) {
    for (const auto &x : files) {
        const auto &name = x.first;
        const auto &file = x.second;
        if(name != "file") continue;
        auto file_content = req.body.substr(file.offset, file.length);
        ofstream output( "test_save_.jpg", ios::out | ios::binary );
        if( ! output )
        {
            cerr << "Open output file error!" << endl;
            return;
        }
        
        output.write ((char *) file_content.c_str(), file.length );
        
        /*unsigned char *rgb_buf = (unsigned char *)malloc(4096*2160*3);
         
         int w=0;int h=0; int c=3;
         
         BoonJpegCodec bjc;
         clock_t t=clock();
         bool ret = bjc.JpegUnCompress((char *) file_content.c_str(), file.length, (char *)rgb_buf,
         4096*2160*3, w, h, c);
         clock_t tt = clock() - t;
         std::cout << "Jpeg w:\t" << w <<"\th:\t" << h << "Time:\t" << tt << std::endl;
         */
        output.close();
    }
    
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
        save_files(http_info.req, http_info.req.files);
        auto body = "{\"code\":0}";
        http_info.res.set_content(body, "application/json");
    }
}
PVPR pvpr;
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
    ServerConf server_conf;
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
            file_content = req.body.substr(file.offset, file.length);
            file_name = file.filename;
            file_len = (int)file.length;
            have_jpg = true;
        }
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
                json_result["duration"]=(int)((clock()-t)/1000);
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
        }
        std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
        jsonWriter->write(json_res, &os);
        body = os.str();
        res.set_content(body, "application/json");
    });
    
    /*svr.set_logger(
     [](const Request &req, const Response &res) { cout << log(req, res); });*/
    //thread_http_handler = boost::thread(boost::bind(&task_http_handler));
  
    cout << "The server started at port " << server_conf.server_port << "..." << endl;
    
    svr.listen(server_conf.server_ip.c_str(), server_conf.server_port);
    
    return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
