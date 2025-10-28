#pragma once

#include "interface/ComEndpoints.hpp"
#include "interface/ComInterface.hpp"

#include "CustomTypes.hpp"
#include "Math.hpp"

namespace eduart {

namespace sensor {

class BaseSensor : public com::ComObserver {
public:
  BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable);
  ~BaseSensor();

  std::size_t getIdx() const;

  bool gotNewData() const;
  bool newDataAvailable() const;

  bool isEnabled() const;
  void setEnable(bool enable);

  void setPose(math::Vector3 translation, math::Vector3 rotation);

  void clearDataFlag();
  virtual void onClearDataFlag() = 0;

  void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;
  virtual void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) = 0;

protected:
  std::size_t _idx;
  SensorState _error;
  com::ComEndpoint _target;
  com::ComInterface* _interface;

  math::Vector3 _translation;
  math::Vector3 _rotation;
  math::Matrix3 _rot_m;

  bool _enable_flag;
  bool _new_data_available_flag;
  bool _new_data_in_buffer_flag;
  bool _new_measurement_ready_flag;
};

} // namespace sensor

} // namespace eduart