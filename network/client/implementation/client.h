#ifndef CLIENT_H
#define CLIENT_H

#include "client/interface/client.h"

class Client : public IClient
{
public:
  Client();

  // IClient interface
public:
  void sendRequest() const;
  void initialize();
  void start();
  void onReceive();
};

#endif // CLIENT_H
