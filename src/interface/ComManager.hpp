#pragma once

#include <deque>
#include <memory>
#include <string>

#include "sensorring/interface/ComInterfaceID.hpp"

#include "ComInterface.hpp"

namespace eduart {

namespace sensorring {

namespace com {

class ComManager {
public:
  ComManager(const ComManager&)            = delete;
  ComManager& operator=(const ComManager&) = delete;

  /**
   * @brief Get a reference to the instance of the ComManager singleton
   * @return Pointer to the ComManager instance
   */
  static ComManager* getInstance() noexcept;

  /**
   * Create or retrieve a communication interface
   * @param[in] id ID of the interface to create or retrieve
   * @param[in] create_if_unknown If true, create the interface if it does not exist
   * @return Raw pointer to the ComInterface, or nullptr if the interface does not exist and create_if_unknown is false
   * @note The returned pointer is valid for the lifetime of the ComManager singleton instance.
   */
  ComInterface* getInterface(com::ComInterfaceID id, bool create_if_unknown = true);

  /**
   * Get all communication interfaces
   * @return Vector of all communication interfaces
   */
  std::vector<ComInterface*> getInterfaces();

private:
  /// Private constructor. The ComManager is a singleton.
  ComManager() = default;

  std::deque<std::unique_ptr<ComInterface> > _interfaces;
};

} // namespace com

} // namespace sensorring

} // namespace eduart