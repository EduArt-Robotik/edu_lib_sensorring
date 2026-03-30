// Copyright (c) 2026 EduArt Robotik GmbH

/**
 * @file   ComEndpoint.hpp
 * @author EduArt Robotik GmbH
 * @brief  Communication endpoint identifier.
 * @date   2026-02-19
 */

#pragma once

#include <string>
#include <unordered_set>

#include "sensorring/platform/SensorringExport.hpp"

namespace eduart {

namespace com {

/**
 * @class ComEndpoint
 * @brief Uniquely identifies a communication endpoint by its string ID.
 */
class SENSORRING_EXPORT ComEndpoint {
public:
  /**
   * @brief Construct from endpoint ID string.
   * @param[in] id The unique identifier for this endpoint.
   */
  ComEndpoint(const std::string& id);

  /// Copy constructor.
  ComEndpoint(const ComEndpoint& endpoint);

  /**
   * @brief Get the endpoint ID.
   * @return The endpoint's unique identifier string.
   */
  const std::string getId() const;

  // Equality operator for ComEndpoint
  bool operator==(const ComEndpoint& other) const;

private:
  const std::string _id;
};

} // namespace com

} // namespace eduart

namespace std {

/**
 * @struct std::hash<ComEndpoint>
 * @brief Hash specialization for ComEndpoint to enable use in unordered containers.
 */
template <> struct hash<eduart::com::ComEndpoint> {
  /**
   * @brief Compute hash value for a ComEndpoint.
   * @param[in] ep The endpoint to hash.
   * @return Hash of the endpoint's ID.
   */
  std::size_t operator()(const eduart::com::ComEndpoint& ep) const { return std::hash<std::string>{}(ep.getId()); }
};

} // namespace std