#pragma once

#include "omnidirectional_layer/interface/IBlockDecoder.h"
#include <memory>

class IClient {
protected:
  std::unique_ptr<icov::omnidirectional_layer::IBlockDecoder> m_decoder;

public:
  virtual ~IClient() {}
  virtual void sendRequest() const = 0;
  virtual void initialize() = 0;
  virtual void start() = 0;
  virtual void onReceive() = 0;
  void setDecoder(icov::omnidirectional_layer::IBlockDecoder *);
  icov::omnidirectional_layer::IBlockDecoder &getDecoder() const;
};
