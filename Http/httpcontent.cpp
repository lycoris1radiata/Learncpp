#include "httpcontent.h"

#include <algorithm>

#include "httprequest.h"
#include "httpparsestate.h"

using namespace tiny_muduo;

HttpContent::HttpContent() : parse_state_(kParseRequestLine)
{
}

HttpContent::~HttpContent()
{
}

bool HttpContent::ParseContent(Buffer *buffer)
{
    bool linemore = true;
    bool parseok = true;
    const char *CRLF = nullptr;
    while (linemore)
    {
        CRLF = nullptr;
        if (parse_state_ == kParseRequestLine)
        {
            CRLF = buffer->FindCRLF();
            if (CRLF)
            {
                parseok = request_.ParseRequestLine(buffer->Peek(), CRLF);
                if (parseok)
                {
                    parse_state_ = kParseHeaders;
                }
                else
                {
                    linemore = false;
                }
            }
            else
            {
                linemore = false;
            }
        }
        else if (parse_state_ == kParseHeaders)
        {
            CRLF = buffer->FindCRLF();
            if (CRLF)
            {
                const char *colon = std::find((const char *)buffer->Peek(), CRLF, ':');
                if (colon == CRLF)
                {
                    if (request_.method() == kGet)
                    {
                        parse_state_ = kParseGotCompleteRequest;
                        linemore = false;
                    }
                    else
                    {
                        parse_state_ = kParseBody;
                    }
                }
                else
                {
                    parseok = request_.ParseHeaders(buffer->Peek(), colon, CRLF);
                    if (!parseok)
                        linemore = false;
                }
            }
            else
            {
                linemore = false;
            }
        }
        else if (parse_state_ == kParseGotCompleteRequest)
        {
            linemore = false;
        }
        else if (parse_state_ == kParseBody)
        {
            if(buffer->Peek()==buffer->beginwrite()){
                parse_state_ = kParseGotCompleteRequest;
                linemore = false;
            }
            else{
                parseok=request_.ParseBody(buffer->Peek(),buffer->beginwrite());
                linemore = false;
                parse_state_ = kParseGotCompleteRequest;
                buffer->RetrieveUntilIndex(buffer->beginwrite());
            }
        }
        if (CRLF)
        {   
            buffer->RetrieveUntilIndex(CRLF + 2);
        }
    }
    return parseok;
}
