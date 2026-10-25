#include "client.h"

#include "common.h"
#include "omnidirectional_layer/interface/IBlockDecoder.h"

void IClient::setDecoder(IBlockDecoder * d)
{
  m_decoder = std::unique_ptr<IBlockDecoder>(d);
}

icov::omnidirectional_layer::IBlockDecoder &IClient::getDecoder() const
{
  return *m_decoder;
}
