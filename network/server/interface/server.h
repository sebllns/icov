#ifndef ISERVER_HPP
#define ISERVER_HPP

#include "omnidirectional_layer/interface/IBlockProcessor.h"
#include <memory>

typedef IBlockProcessor ISinglePassDecoder;

class IServer {
protected:
  std::unique_ptr<ISinglePassDecoder> m_extractor;

public:
  virtual ~IServer() {}
  virtual void sendBitstream() const = 0;
  virtual void initialize() = 0;
  virtual void start() = 0;
  virtual void onReceive() = 0;
  void setExtractor(ISinglePassDecoder *);
  const ISinglePassDecoder &getExtractor() const;
};

#endif // ISERVER_HPP
