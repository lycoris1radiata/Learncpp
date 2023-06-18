#include "httpserver.h"

#include <functional>
#include <string.h>
#include "address.h"
#include "sql_connection_pool.h"
#include "log.h"

using namespace tiny_muduo;
using tiny_muduo::Version;

HttpServer::HttpServer(EventLoop *loop, const Address &address, bool auto_close_idleconnection,bool isSql) : loop_(loop), server_(loop, address), auto_close_idleconnection_(auto_close_idleconnection),isSql_(isSql)
{
    server_.SetConnectionCallback(std::bind(&HttpServer::ConnectionCallback, this, _1));
    server_.SetMessageCallback(std::bind(&HttpServer::MessageCallback, this, _1, _2));
    // SetHttpResponseCallback(std::bind(&HttpServer::HttpDefaultCallback, this, _1, _2));
    LOG_INFO ("Httpserver listening on %s : %d\n",address.ip(),address.port());
}

HttpServer::~HttpServer()
{
}

void HttpServer::Start(){

    sqlconnpool_ = connection_pool::GetInstance();
    sqlconnpool_->init("localhost", "root", "zzx12345", "yourdb", 3306, 8);
    initmysql_result(sqlconnpool_);

    server_.Start();
    }

void HttpServer::MessageCallback(const ConnectionPtr &connection, Buffer *buffer)
{
    HttpContent *content = connection->GetHttpContent();
    if (auto_close_idleconnection_)
        connection->UpdateTimestamp(Timestamp::Now());
    if (connection->IsShutDown())
        return;

    if (!content->ParseContent(buffer))
    {
        connection->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        connection->Shutdown();
    }

    if (content->GetCompleteRequest())
    {
        DealWithRequest(content->request(), connection);
        content->ResetContentState();
    }
}

void HttpServer::DealWithRequest(const HttpRequest &request, const ConnectionPtr &connection)
{
    string connection_state = std::move(request.GetHeader("Connection"));
    bool close = (connection_state == "Close" ||
                  (request.version() == kHttp10 &&
                   connection_state != "Keep-Alive"));

    HttpResponse response(close);

    MYSQL* mysql=NULL;
    if(isSql_)
        connectionRAII mysqlcon(&mysql, sqlconnpool_);

    Response(request, response,mysql);

    Buffer buffer;
    response.AppendToBuffer(&buffer);

    //string tmp(buffer.beginread(),buffer.readablebytes());
    //LOG_INFO("send message:\n%s\n",&*tmp.begin());
    
    connection->Send(&buffer);

    if (response.CloseConnection())
    {
        connection->Shutdown();
    }
}

void HttpServer::Response(const HttpRequest& request, HttpResponse& response,MYSQL* mysql) {
      
  const string& path = request.path().substr(0,2); 

  if (path == "/") {
    response.SetStatusCode(k200OK);
    response.SetBodyType("text/html");
    response.SetHTMLpath("judge.html");
    return;
  }

  else if ( (path == "/2" ||path == "/3")&&request.CGI()==1)
  {
      // 将用户名和密码提取出来
      // user=123&passwd=123
      char name[100], password[100];
      std::string m_string=request.mystr();
      int i;
      for (i = 5; m_string[i] != '&'; ++i)
          name[i - 5] = m_string[i];
      name[i - 5] = '\0';

      int j = 0;
      for (i = i + 10; m_string[i] != '\0'; ++i, ++j)
          password[j] = m_string[i];
      password[j] = '\0';

      response.SetStatusCode(k200OK);
      response.SetBodyType("text/html");

      if (path == "/3")
      {
          // 如果是注册，先检测数据库中是否有重名的
          // 没有重名的，进行增加数据
          char *sql_insert = (char *)malloc(sizeof(char) * 200);
          strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
          strcat(sql_insert, "'");
          strcat(sql_insert, name);
          strcat(sql_insert, "', '");
          strcat(sql_insert, password);
          strcat(sql_insert, "')");

          if (users.find(name) == users.end())
          {
              MutexLock mutex;
              int res;
              {
                  MutexLockGuard lock(mutex);
                  res = mysql_query(mysql, sql_insert);
                  users.insert(std::pair<string,string>(name, password));
              }
  
              if (!res){
                  response.SetHTMLpath("log.html");
              }
              else
                  response.SetHTMLpath("registerError.html");
          }
          else
              response.SetHTMLpath("registerError.html");
      }
      // 如果是登录，直接判断
      // 若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
      else if (path == "/2")
      {
          if (users.find(name) != users.end() && users[name] == password)
              response.SetHTMLpath("welcome.html");
          else
              response.SetHTMLpath("logError.html");
      }
  }

  else if(path =="/0"){
    response.SetStatusCode(k200OK);
    response.SetBodyType("text/html");
    response.SetHTMLpath("register.html");

  }
  else if(path =="/1"){
    response.SetStatusCode(k200OK);
    response.SetBodyType("text/html");
    response.SetHTMLpath("log.html");
  }
  else if(path =="/5"){
    response.SetStatusCode(k200OK);
    response.SetBodyType("text/html");
    response.SetHTMLpath("picture.html");

  }
  else if(path =="/6"){
    response.SetStatusCode(k200OK);
    response.SetBodyType("text/html");
    response.SetHTMLpath("video.html");

  }
  else {
    response.SetStatusCode(k400BadRequest);
    response.SetStatusMessage("Bad Request");
    response.SetCloseConnection(true);
    return;
  }
}

void HttpServer::initmysql_result(connection_pool *connPool)
{
    // 先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);

    // 在user表中检索username，passwd数据，浏览器端输入
    // if(mysql_query(mysql, "SELECT username,passwd FROM user")){
    //     printf("mysql select error\n");
    // }
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    // 从表中检索完整的结果集
    MYSQL_RES *result = ::mysql_store_result(mysql);

    // 返回结果集中的列数
    // int num_fields = ::mysql_num_fields(result);

    // 返回所有字段结构的数组
    // MYSQL_FIELD *fields = ::mysql_fetch_fields(result);

    // 从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = ::mysql_fetch_row(result))
    {
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        users[temp1] = temp2;
    }
}