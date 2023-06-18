#include <iostream>
#include <stdio.h>
#include <string>
#include <functional>
#include <memory>
#include <climits>
#include <map>
#include <string.h>

#include "EventLoop.h"
#include "Server.h"
#include "address.h"
#include "httpserver.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "mutex"
#include "log.h"

#include <mysql/mysql.h>

using namespace tiny_muduo;
using tiny_muduo::HttpStatusCode;

int close_log=1;

int main()
{
    Log::get_instance()->init("./LOGFILE/",2000,800000,0);
    EventLoop loop;
    Address listen_addr(1234);
    HttpServer server(&loop,listen_addr,false,true);
    server.SetThreadNums(4);
    server.Start();
    loop.loop();
    return 0;
}