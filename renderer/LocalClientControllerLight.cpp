#include "LocalClientControllerLight.h"

#include "helpers.h"
#include "omnidirectional_layer/Povrequest.h"
#include "omnidirectional_layer/geometry.h"
#include "omnidirectional_layer/implementation/BlockOrdering.h"
#include "omnidirectional_layer/implementation/EquiRectPicture.h"
#include "omnidirectional_layer/implementation/IcovAccessBlockManager.h"
#include "omnidirectional_layer/implementation/ViewportManagerCenterPadded.h"
#include "omnidirectional_layer/interface/IBlockProcessor.h"

LocalClientControllerLight::LocalClientControllerLight(
    ProcessBlock pb, ProcessEvent start, ProcessEvent end, int videoWidth,
    int videoHeight, int block_size, int screenWidth, int screenHeight,
    float fovX_radian) {

  std::cout << videoWidth << " " << videoHeight << " " << block_size << " "
            << screenWidth << " " << screenHeight << " "
            << (fovX_radian * RAD2DEG) << std::endl;

  m_request = new PoVRequest();

  auto picture = std::make_shared<EquiRectPicture>(videoWidth, videoHeight);
  picture->buildConstantSizeBlocks(block_size, block_size);
  req_mgr.setPicture(picture);
  req_mgr.setAccessBlocks(std::make_shared<IcovAccessBlockManager>());
  req_mgr.setOrdering(std::make_shared<BlockOrdering>(picture->icus().size()));
  req_mgr.setViewport(std::make_shared<ViewportManagerCenterPadded>(1.2f));
  req_mgr.setBlockPrediction(std::make_shared<BlockPrediction>());

  float fovY_radian = 2 * atan(tan(fovX_radian * 0.5) *
                               ((float)screenHeight / ((float)screenWidth)));

  std::cout << (fovY_radian * RAD2DEG) << std::endl;

  // Define the viewport static geometry
  vpg.focale = 1 / (2.0f * tan(fovX_radian * 0.5)); // focale
  vpg.center = point2d(0.5f, 0.5f);                 // center normalised coords
  vpg.fov = angle_radian(fovX_radian, fovY_radian); // field of view in radians
  vpg.resolution = point2d(screenWidth, screenHeight); // resolution in pixels

  // viewport geometry in cartesian coords
  point2d viewport_size(abs(2 * tan(0.5 * fovX_radian)),
                        abs(2 * tan(0.5 * fovY_radian)));

  vpg.roi_size = viewport_size;
  req_mgr.viewport()->viewportGeometry() = vpg;

  m_pblocksProcessor = std::make_shared<BlockProcessor>(pb, start, end);
  m_pblocksProcessor->blockIds.reserve(2 * picture->icus().size());
  req_mgr.setBlockProcessor(m_pblocksProcessor);
}

LocalClientControllerLight::~LocalClientControllerLight() {
  if (m_request != nullptr) {
    delete m_request;
    m_request = nullptr;
  }
}

void LocalClientControllerLight::start() { req_mgr.start(); }

void LocalClientControllerLight::update() {
  req_mgr.update(m_request->getAngleX(), m_request->getAngleY());
}

void LocalClientControllerLight::setPoV(float xAngle, float yAngle) {
  m_request->setAngleX(xAngle);
  m_request->setAngleY(yAngle);
}

void LocalClientControllerLight::getAllBlocks(ProcessBlock pb) const {
  ICOV_ASSERT_MSG(pb != nullptr, "null callback");
  for (const auto &b : req_mgr.picture()->icus()) {
    pb(static_cast<uint32_t>(b.getId(Block::SELF)), b.u(), b.v(), b.w(), b.h(),
       0);
  }
}

uint32_t LocalClientControllerLight::getBlocksCount() const {
  return req_mgr.picture()->icus().size();
}

void LocalClientControllerLight::getVisibleBlocks(int *&blocks,
                                                  int &count) const {
  blocks = m_pblocksProcessor->blockIds.data();
  count = (int)m_pblocksProcessor->blockIds.size();
}

// const std::vector<int> &LocalClientControllerLight::visibleBlocks() const
//{
//    return m_pblocksProcessor->blockIds;
//}

LocalClientControllerLight::BlockProcessor::BlockProcessor(ProcessBlock pb,
                                                           ProcessEvent start,
                                                           ProcessEvent end)
    : m_processBlockCallback(pb), m_startCallback(start), m_endCallback(end) {
  ICOV_ASSERT_MSG(m_processBlockCallback != nullptr, "null callback");
  ICOV_ASSERT_MSG(m_startCallback != nullptr, "null callback");
  ICOV_ASSERT_MSG(m_endCallback != nullptr, "null callback");
}

void LocalClientControllerLight::BlockProcessor::startProcess() {
  // m_startCallback();
  blockIds.clear();
}

void LocalClientControllerLight::BlockProcessor::processBlock(
    const Block &b, IBlockPredictionManager::prediction_t prediction) {
  blockIds.push_back(static_cast<int>(b.getId(Block::SELF)));
  blockIds.push_back(static_cast<int>(prediction));
  // m_processBlockCallback(static_cast<uint32_t>(b.getId(Block::SELF)), b.u(),
  // b.v(), b.w(), b.h(), prediction);
}

void LocalClientControllerLight::BlockProcessor::endProcess() {
  // m_endCallback();
}
