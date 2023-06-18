#pragma once

#include <stdio.h>
#include <functional>
#include <utility>
#include <memory>
#include <map>
#include <string>

#include "callback.h"
#include "EventLoop.h"
#include "Server.h"
#include "Connection.h"
#include "Buffer.h"
#include "httpresponse.h"
#include "httpcontent.h"
#include "httprequest.h"
#include "timestamp.h"
#include "sql_connection_pool.h"

#include <mysql/mysql.h>

using tiny_muduo::HttpStatusCode;

namespace tiny_muduo
{
static const double kIdleConnectionTimeOuts=8.0;

class HttpServer{

public:
    HttpServer(EventLoop* loop,const Address& address,bool auto_close_idleconnection=false,bool isSql=true);
    ~HttpServer();

    void Start();

    void HttpDefaultCallback(const HttpRequest& request,HttpResponse& response){
        response.SetStatusCode(k404NotFound);
        response.SetStatusMessage("Not Found");
        response.SetCloseConnection(true);
        (void)request;
    }
    
    void HandleIdleConnection(std::weak_ptr<Connection>& connection){
        ConnectionPtr conn(connection.lock());
        if(conn){
            if(Timestamp::AddTime(conn->timestamp(),kIdleConnectionTimeOuts)<Timestamp::Now()){
                conn->Shutdown();
            }else{
                loop_->RunAfter(kIdleConnectionTimeOuts, 
                            std::move(std::bind(&HttpServer::HandleIdleConnection,
                            this, connection)));
            }
        }
    }
    void ConnectionCallback(const ConnectionPtr& connection){
        if (auto_close_idleconnection_) {
        loop_->RunAfter(kIdleConnectionTimeOuts, 
                        std::bind(&HttpServer::HandleIdleConnection, 
                        this, std::weak_ptr<Connection>(connection))); 
        }
    }
    void MessageCallback(const ConnectionPtr& connection, Buffer* buffer);
    
    void SetThreadNums(int thread_nums) {server_.SetThreadNums(thread_nums);}

    void DealWithRequest(const HttpRequest& request, const ConnectionPtr& connection);

    void Response(const HttpRequest& request, HttpResponse& response,MYSQL* mysql);

    void initmysql_result(connection_pool *connPool);


private:
    EventLoop* loop_;
    Server server_;
    bool auto_close_idleconnection_;

    connection_pool *sqlconnpool_;
    bool isSql_;
    std::map<std::string, std::string> users;
};
} // namespace tiny_muduo
