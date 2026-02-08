#pragma once

#include <deque>
#include <memory>
#include <string>

#include "sensorring/types/InterfaceType.hpp"
#include "types/SingletonTemplate.hpp"

#include "ComInterface.hpp"

namespace eduart {

namespace com {

class ComManager : public Singleton<ComManager> {
public:
  ComManager(const ComManager&)            = delete;
  ComManager& operator=(const ComManager&) = delete;

  /**
   * Create or retrieve a communication interface
   * @param[in] interface_name Name of the interface to create or retrieve
   * @param[in] type Type of the interface to create
   * @return Raw pointer to the ComInterface, or nullptr on failure
   * @note The returned pointer is valid for the lifetime of the ComManager singleton instance.
   */
  ComInterface* createInterface(std::string interface_name, InterfaceType type);

  /**
   * Retrieve an existing communication interface by name
   * @param[in] interface_name Name of the interface to retrieve
   * @return Raw pointer to the ComInterface if found, or nullptr if not found
   * @note The returned pointer is valid for the lifetime of the ComManager singleton instance.
   */
  ComInterface* getInterface(std::string interface_name);

  /**
   * Get all communication interfaces
   * @return Vector of all communication interfaces
   */
  std::vector<ComInterface*> getInterfaces();

private:
  friend class Singleton<ComManager>;
  ComManager() = default;

  std::deque<std::unique_ptr<ComInterface> > _interfaces;
};

} // namespace com

} // namespace eduart