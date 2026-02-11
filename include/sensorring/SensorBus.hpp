#pragma once

#include <memory>
#include <vector>

#include "sensorring/interface/ComObserver.hpp"

#include "SensorBoard.hpp"

namespace eduart {

namespace com {
class ComInterface;
}

namespace bus {

class SensorBus : public com::ComObserver {
public:
  SensorBus(com::ComInterface* interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec);
  ~SensorBus();

  size_t getSensorCount() const;
  size_t getEnumerationCount() const;
  const std::vector<device::EnumerationInformation>& getEnumerationInfo() const;

  com::ComInterface* getInterface() const;

  std::vector<device::SensorBoard*> getSensorBoards() const;

  int enumerateDevices();

  void setBrs(bool brs_enable);

  void comCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

private:
  com::ComInterface* _interface;
  std::vector<device::EnumerationInformation> _enumeration_vec;
  std::vector<std::unique_ptr<device::SensorBoard> > _board_vec;

  std::atomic<bool> _enumeration_flag;
  std::atomic<unsigned int> _enumeration_count;
};

} // namespace bus

} // namespace eduart