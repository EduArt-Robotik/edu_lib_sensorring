#pragma once

#include "interface/ComInterface.hpp"
#include "sensorring/interface/ComEndpoint.hpp"
#include "sensorring/math/Matrix3.hpp"

namespace eduart {

namespace device {

enum class SENSORRING_EXPORT SensorState {
  SensorInit,
  SensorOK,
  ReceiveError
};

class BaseSensor : public com::ComObserver {
public:
  BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable);
  ~BaseSensor();

  std::size_t getIdx() const;

  bool gotNewData() const;
  bool newDataAvailable() const;

  bool getEnable() const;
  void setEnable(bool enable);

  void setPose(math::Vector3 translation, math::Vector3 rotation);

  void resetSensorState();
  void clearDataFlag();

protected:
  virtual void onResetSensorState() = 0;
  virtual void onClearDataFlag()    = 0;

  std::size_t _idx;
  SensorState _error;
  com::ComInterface* _interface;

  math::Vector3 _translation;
  math::Vector3 _rotation;
  math::Matrix3 _rot_m;

  bool _enable_flag;
  bool _new_data_available_flag;
  bool _new_data_in_buffer_flag;
  bool _new_measurement_ready_flag;
};

} // namespace device

} // namespace eduart