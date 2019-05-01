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
#include "common.h"
#include "httplib.h"
#include "concurrent_queue.h"

using namespace httplib;
using namespace std;

class HttpInfo
{
public:
    Request req;
    Response res;
};

concurrent_queue<HttpInfo> g_http_buff;

boost::thread thread_http_handler;

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

int main(int argc, const char **argv) {
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
        HttpInfo http_info;
        http_info.req = req;
        http_info.res = res;
        g_http_buff.push(http_info);
        //auto body = "{\"code\":0}";
        //res.set_content(body, "application/json");
    });
    
    /*svr.set_logger(
     [](const Request &req, const Response &res) { cout << log(req, res); });*/
    thread_http_handler = boost::thread(boost::bind(&task_http_handler));
  
    cout << "The server started at port " << server_conf.server_port << "..." << endl;
    
    svr.listen(server_conf.server_ip.c_str(), server_conf.server_port);
    
    return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
