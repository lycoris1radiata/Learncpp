#include "httpresponse.h"

#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"
#include "Buffer.h"

using namespace tiny_muduo;
using std::string;
const string HttpResponse::server_name_="ZZX";

void HttpResponse::AppendToBuffer(Buffer *buffer)
{
    char buf[32]={0};
    snprintf(buf,sizeof(buf),"HTTP/1.1 %d ",status_code_);
    buffer->Append(buf);
    buffer->Append(status_message_);
    buffer->Append(CRLF);
    if(close_connection_){
        buffer->Append("Connection:close\r\n");
    }else{
        snprintf(buf,sizeof(buf),"Content-Length: %ld\r\n",m_file_stat.st_size);
        buffer->Append(buf);
        buffer->Append("Connection: Keep-Alive\r\n");
    }
    buffer->Append("Content-Type: ");
    buffer->Append(type_);
    buffer->Append(CRLF);

    buffer->Append("Server: ");
    buffer->Append(server_name_);
    buffer->Append(CRLF);
    buffer->Append(CRLF);

    //string tmp(buffer->beginread(),buffer->readablebytes());
    //LOG_INFO("send message:\n%s\n",&*tmp.begin());

    int fd = open(&*file_path_.begin(),O_RDONLY);
    assert(fd!=-1);
    buffer->ReadFd(fd);
    close(fd);
    
    return;
}