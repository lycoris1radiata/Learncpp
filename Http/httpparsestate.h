#pragma once

namespace tiny_muduo {

enum HttpRequestParseState {
  kParseRequestLine,
  kParseHeaders,
  kParseBody,
  kParseGotCompleteRequest,
  kParseErrno,
};

}
