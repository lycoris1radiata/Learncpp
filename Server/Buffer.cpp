#include "Buffer.h"
#include <sys/uio.h>
#include <iostream>

#include "log.h"

int tiny_muduo::Buffer::ReadFd(int fd)
{
    char extrabuf[65536] = {0};
    struct iovec iv[2];
    const int writable = writablebytes();
    iv[0].iov_base = beginwrite();
    iv[0].iov_len = writable;
    iv[1].iov_base = extrabuf;
    iv[1].iov_len = sizeof(extrabuf);
    const int iovcnt = (writable < static_cast<int>(sizeof(extrabuf)) ? 2 : 1);
    int readn = static_cast<int>(::readv(fd, iv, iovcnt));
    if (readn < 0)
    {
        LOG_WARN("%s\n","Buffer::ReadFd readv failed");
    }
    else if (readn <= writable)
    {
        write_index_ += readn;
    }
    else
    {
        write_index_ = static_cast<int>(buffer_.size());
        Append(extrabuf, readn - writable);
    }

    //std::string tmp(beginread(),beginread()+readablebytes());
    //std::cout<<tmp<<std::endl;

    return readn;
}