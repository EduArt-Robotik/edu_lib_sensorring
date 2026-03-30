#include "HTPA32_Eeprom.hpp"

namespace eduart {

namespace device {

namespace htpa32 {

std::size_t HTPA32_Eeprom::serialize(uint8_t* buffer, std::size_t buffer_size) {
  return htpa32_serialize(&data, buffer, buffer_size);
}

std::optional<HTPA32_Eeprom> HTPA32_Eeprom::deserialize(const uint8_t* buffer, std::size_t buffer_size) {
  HTPA32_Eeprom eeprom;
  if (htpa32_deserialize(&eeprom.data, buffer, buffer_size) == 0) {
    return eeprom;
  }
  return std::nullopt;
}

} // namespace htpa32

} // namespace device

} // namespace eduart