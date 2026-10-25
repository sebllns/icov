#include "clientcontroller.h"

#include "client/implementation/client.h"
#include "omnidirectional_layer/Povrequest.h"
#include "omnidirectional_layer/implementation/BlockDecoder.h"
#include "renderer.h"

ClientController::ClientController() {
  m_request = new PoVRequest();
  m_client = std::unique_ptr<IClient>(new Client());

  IBlockDecoder *decoder = new BlockDecoder();
  IRenderer *renderer = new Renderer();

  renderer->setRequest(m_request);
  decoder->setRenderer(renderer);
  m_client->setDecoder(decoder);
}

ClientController::~ClientController() {
  // delete m_request;
}

void ClientController::start() {
  m_client->initialize();
  m_client->start();
}

void ClientController::update() {
  m_client->sendRequest();
  m_client->onReceive();
}

void ClientController::setPoV(float xAngle, float yAngle) {
  m_request->setAngleX(xAngle);
  m_request->setAngleY(yAngle);
}

const IClient &ClientController::getClient() const { return *m_client; }
