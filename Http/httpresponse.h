#pragma once

#include <string>
#include <utility>
#include <sys/stat.h>

#include "httprequest.h"

using std::string;

namespace tiny_muduo
{
static const string CRLF="\r\n";
enum HttpStatusCode {
  k100Continue = 100,
  k200OK = 200,
  k400BadRequest = 400,
  k403Forbidden = 403,
  k404NotFound = 404,
  k500InternalServerErrno = 500
};

class Buffer;
class HttpResponse{
public:
    HttpResponse(bool close_connection):type_("text/plain"),close_connection_(close_connection),file_path_("./root/"){}
    ~HttpResponse(){}
    void SetStatusCode(HttpStatusCode status_code){status_code_=status_code;}
    void SetStatusMessage(const string& status_message){status_message_=status_message;}
    void SetCloseConnection(bool close_connection) { close_connection_ = close_connection; }
    void SetBodyType(const string& type) { type_ = type; }
    void SetBody(const string& body) { body_ = body; }
    void AppendToBuffer(Buffer* buffer);
    bool CloseConnection() { return close_connection_; }
    
    void SetHTMLpath(const string& path){
          file_path_+=path;
          stat(&*file_path_.begin(), &m_file_stat);}
private:
    static const string server_name_;
    HttpStatusCode status_code_;
    string status_message_;
    string body_;
    string type_;
    bool close_connection_;
    
    string file_path_;
    struct stat m_file_stat;
};
} // namespace tiny_muduo
