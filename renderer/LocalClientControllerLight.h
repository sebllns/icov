#pragma once

#include <memory>

#include "omnidirectional_layer/RequestManager.h"
#include "omnidirectional_layer/interface/IBlockPredictionManager.h"
#include "omnidirectional_layer/interface/IBlockProcessor.h"

class PoVRequest;

// callback for blocks
extern "C" typedef void (*ProcessBlock)(
    uint32_t id, float u, float v, float w, float h,
    icov::IBlockPredictionManager::prediction_t prediction);

extern "C" typedef void (*ProcessEvent)();

class LocalClientControllerLight {
  class BlockProcessor : public IBlockProcessor {
    // IBlockProcessor interface
  public:
    BlockProcessor(ProcessBlock pb, ProcessEvent start, ProcessEvent end);

    void startProcess() override;
    void processBlock(
        const Block &b,
        icov::IBlockPredictionManager::prediction_t prediction) override;
    void endProcess() override;

  protected:
    ProcessBlock m_processBlockCallback;
    ProcessEvent m_startCallback;
    ProcessEvent m_endCallback;

  public:
    std::vector<int> blockIds;
  };

protected:
  PoVRequest *m_request;
  std::shared_ptr<BlockProcessor> m_pblocksProcessor;
  RequestManager req_mgr;
  viewport_geometry vpg;

public:
  LocalClientControllerLight(ProcessBlock pb, ProcessEvent start,
                             ProcessEvent end, int videoWidth, int videoHeight,
                             int block_size, int screenWidth, int screenHeight,
                             float fovX_radian);
  ~LocalClientControllerLight();
  void start();
  void update();
  void setPoV(float xAngle, float yAngle);
  void getAllBlocks(ProcessBlock pb) const;
  uint32_t getBlocksCount() const;
  void getVisibleBlocks(int *&blocks, int &count) const;

  //  const std::vector<int>& visibleBlocks() const;
};
