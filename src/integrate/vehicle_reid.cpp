//
//  simplesvr.cc
//
//  Copyright (c) 2013 Yuji Hirose. All rights reserved.
//  The Boost Software License 1.0
//

#include <cstdio>
#include <httplib.h>
#include <iostream>
#include <fstream>
#include <json/json.h>

#include "jpg_codec.h"

using namespace httplib;
using namespace std;

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
        ofstream output( "test_save.jpg", ios::out | ios::binary );
        if( ! output )
        {
            cerr << "Open output file error!" << endl;
            return;
        }
        
        output.write ((char *) file_content.c_str(), file.length );
        
        char

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

int main(int argc, const char **argv) {
  Server svr;

  svr.Post("/chpAnalyze", [](const Request &req, Response &res) {
      //auto body = dump_headers(req.headers) + dump_multipart_files(req.files);
      save_files(req, req.files);
      auto body = "{\"code\":0}";
      res.set_content(body, "application/json");
  });

  svr.set_logger(
      [](const Request &req, const Response &res) { cout << log(req, res); });

  auto port = 8080;
  if (argc > 1) { port = atoi(argv[1]); }

  auto base_dir = "./";
  if (argc > 2) { base_dir = argv[2]; }

  svr.set_base_dir(base_dir);

  cout << "The server started at port " << port << "...";

  svr.listen("0.0.0.0", port);

  return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
