#pragma once

#include <utility>
#include <algorithm>
#include "Buffer.h"
#include "httprequest.h"
#include "httpparsestate.h"

namespace tiny_muduo{
class HttpContent{
public:
    HttpContent();
    ~HttpContent();
    bool ParseContent(Buffer* buffer);
    bool GetCompleteRequest(){return parse_state_==kParseGotCompleteRequest;}

    const HttpRequest& request()const{return request_;}
    void ResetContentState(){
        HttpRequest tmp;
        request_.Swap(tmp);
        parse_state_=kParseRequestLine;
    }
private:
    HttpRequest request_;
    HttpRequestParseState parse_state_;
};
}