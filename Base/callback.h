#pragma once

#include <memory>
#include <functional>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace tiny_muduo {
  class Connection;
  typedef std::shared_ptr<Connection> ConnectionPtr;
  
  class Buffer;
  typedef std::function<void (const ConnectionPtr&, Buffer*)> ConnectionCallback;
  typedef std::function<void (const ConnectionPtr&, Buffer*)> MessageCallback;
  typedef std::function<void ()> ReadCallback;
  typedef std::function<void ()> WriteCallback;
  typedef std::function<void (const ConnectionPtr&)> CloseCallback;
}
