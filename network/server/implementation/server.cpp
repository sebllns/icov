#include "server.h"

#include "common.h"

using namespace std;

Server::Server() { cout << "new Server" << endl; }

void Server::sendRequest() const {
  cout << "Server::sendRequest" << endl;
  // m_decoder->get.serialize();
}

void Server::initialize() { cout << "Server::initialize" << endl; }

void Server::start() { cout << "Server::start" << endl; }

void Server::onReceive() {
  cout << "Server::onReceive" << endl;
  // m_extractor -> decode();
}
