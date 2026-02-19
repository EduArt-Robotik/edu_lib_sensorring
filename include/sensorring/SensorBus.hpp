// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   SensorBus.hpp
 * @author EduArt Robotik GmbH
 * @brief  One communication bus owning multiple sensor boards.
 * @date   2025-02-19
 */

#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "sensorring/interface/ComObserver.hpp"

#include "SensorBoard.hpp"

namespace eduart {

namespace com {
class ComInterface;
}

namespace bus {

/**
 * @class SensorBus
 * @brief One communication interface (e.g. CAN) owning multiple SensorBoards; runs enumeration and forwards COM messages.
 */
class SensorBus : public com::ComObserver {
public:
  /**
   * @brief Construct the bus with a communication interface and owned sensor boards.
   * @param[in] interface Communication interface for this bus.
   * @param[in] board_vec Owned sensor boards.
   */
  SensorBus(com::ComInterface* interface, std::vector<std::unique_ptr<device::SensorBoard> > board_vec);
  /// Destructor
  ~SensorBus();

  /**
   * @brief Total number of sensor boards on this bus.
   * @return Number of boards.
   */
  size_t getSensorCount() const;
  /**
   * @brief Number of boards that have responded during the last enumeration.
   * @return Enumeration response count.
   */
  size_t getEnumerationCount() const;
  /**
   * @brief Enumeration info for each board that responded (order matches response order).
   * @return Const reference to vector of EnumerationInformation.
   */
  const std::vector<device::EnumerationInformation>& getEnumerationInfo() const;

  /**
   * @brief Communication interface used by this bus.
   * @return Pointer to the ComInterface.
   */
  com::ComInterface* getInterface() const;

  /**
   * @brief Non-owning pointers to all sensor boards on this bus.
   * @return Vector of SensorBoard pointers.
   */
  std::vector<device::SensorBoard*> getSensorBoards() const;

  /**
   * @brief Trigger enumeration on this bus and wait for board responses.
   * @return 0 on success, non-zero on failure.
   */
  int enumerateDevices();

  /**
   * @brief Enumerate boards on an interface without pre-created SensorBoards (for AutoDetect creation).
   * Sends the enumeration command and collects all CMD_ACTIVE_DEVICE_RESPONSE replies within a fixed timeout.
   * @param[in] interface Communication interface to enumerate.
   * @return Vector of enumeration info (one per responding board); may be empty if none or on error.
   */
  static std::vector<device::EnumerationInformation> enumerateInterface(com::ComInterface* interface);

  /**
   * @brief Enable or disable bit rate switching on the bus interface.
   * @param[in] brs_enable Enable flag.
   */
  void setBrs(bool brs_enable);

  /**
   * @brief Handle incoming COM message; used for enumeration counts and forwarding to boards.
   * @param[in] source Endpoint that received the message.
   * @param[in] data Raw message payload.
   */
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