#include "server.h"

#include "common.h"

void IServer::setExtractor(ISinglePassDecoder *d) {
  m_extractor = std::unique_ptr<ISinglePassDecoder>(d);
}

const ISinglePassDecoder &IServer::getExtractor() const { return *m_extractor; }
