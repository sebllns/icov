#include "client.h"
#include "common.h"
#include "omnidirectional_layer/interface/IBlockDecoder.h"

using namespace std;
Client::Client()
{
  cout << "new client" << endl;
}

void Client::sendRequest() const
{
  cout << "Client::sendRequest" << endl;
  //m_decoder->get.serialize();
}

void Client::initialize()
{
  cout << "Client::initialize" << endl;
}

void Client::start()
{
  cout << "Client::start" << endl;
}

void Client::onReceive()
{
  cout << "Client::onReceive" << endl;
  m_decoder->decode();
}
