#ifndef CLIENTCONTROLLER_H
#define CLIENTCONTROLLER_H

#include <memory>

class IClient;
class PoVRequest;

class ClientController
{
protected:
  std::unique_ptr<IClient> m_client;
  PoVRequest* m_request;

public:
  ClientController();
  ~ClientController();
  void start();
  void update();
  void setPoV(float xAngle, float yAngle);

  const IClient& getClient() const;
};

#endif // CLIENTCONTROLLER_H
