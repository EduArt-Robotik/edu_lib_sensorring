
#pragma once

#include <chrono>
#include <mutex>

#include "interface/ComInterface.hpp"
#include "sensorring/interface/InterfaceParams.hpp"

using namespace std::chrono_literals;

namespace eduart {

namespace sensorring {

namespace com {

/**
 * @class CanInterface
 * @brief CAN communication base class.
 * @author Hannes Duske
 * @date 17.06.2026
 */
class CanInterface : public ComInterface {
public:
  CanInterface(ComInterfaceID id);

  CanInterface(ComInterfaceID id, const CanParams& params);

  bool configure() override;

  bool setParams(const CanParams& params);

  bool getParams(CanParams& params);

  bool setBrs(bool enable);

  bool getBrs(bool& enable);

  bool setDataRate(unsigned int data_rate);

  bool getDataRate(unsigned int& data_rate);

  bool setDataSamplePoint(float data_sample_point);

  bool getDataSamplePoint(float& data_sample_point);

protected:
  CanParams _params;

private:
  static constexpr std::uint8_t FIRST_BOARD_ADDRESS                = 0;
  static constexpr std::chrono::milliseconds SET_PARAMETER_DELAY   = 10ms;
  static constexpr std::chrono::milliseconds GET_PARAMETER_SLEEP   = 10ms;
  static constexpr std::chrono::milliseconds GET_PARAMETER_TIMEOUT = 100ms;

  void comCallback(const com::ComEndpoint source, std::uint8_t command, const std::vector<uint8_t>& data);

  bool _got_update;
  Mutex _param_mutex;

  subscription::Subscription _com_subscription;
};

} // namespace com

} // namespace sensorring

} // namespace eduart