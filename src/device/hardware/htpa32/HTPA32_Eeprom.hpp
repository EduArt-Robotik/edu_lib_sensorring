#pragma once

#include <cstdint>
#include <optional>

namespace eduart {

namespace device {

namespace htpa32 {

extern "C" {
#include "htpa32_eeprom.h"
}

struct HTPA32_Eeprom {

  /// Serialized size of the EEPROM data in bytes.
  static constexpr std::size_t SERIALIZED_SIZE = HTPA32_EEPROM_SERIALIZED_SIZE;

  /// Raw data from the sensor's EEPROM stored in a struct to access individual fields.
  htpa32_eeprom_t data;

  /**
   *  Serialize the eeprom struct into buffer.
   *  @param[in]  htpa32 array to serialize
   *  @param[out] Buffer to write the serialized data to
   *  @param[in]  Buffer size for safety
   *  @return     Number of bytes written. Returns 0 if the buffer is too small.
   **/
  std::size_t serialize(uint8_t* buffer, std::size_t buffer_size);

  /**
   *  Deserialize the eeprom struct from buffer.
   *  @param[in]  Buffer size for safety
   *  @return     Returns the deserialized HTPA32_Eeprom struct. Returns std::nullopt if the buffer is too small or deserialization fails.
   **/
  static std::optional<HTPA32_Eeprom> deserialize(const uint8_t* buffer, std::size_t buffer_size);
};

} // namespace htpa32

} // namespace device

} // namespace eduart