#ifndef SERVER_H
#define SERVER_H

#include "server/interface/server.h"

class Server : public IServer
{
public:
  Server();

  // IServer interface
public:
  void sendRequest() const;
  void initialize();
  void start();
  void onReceive();
};

#endif // SERVER_H
