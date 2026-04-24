add_library(sensorring_firmware_update STATIC
  FirmwareUpdater.cpp
  internal/BootloaderActivator.cpp
  internal/BootloaderDevice.cpp
  internal/BootloaderProtocol.cpp
  internal/Crc32.cpp
  internal/HexFile.cpp
  ${PROJECT_SOURCE_DIR}/external/frankly_bootloader/src/francor/franklyboot/msg.cpp
)

add_library(sensorring::firmware_update ALIAS sensorring_firmware_update)

target_include_directories(sensorring_firmware_update
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/frankly_bootloader/include>
)

target_link_libraries(sensorring_firmware_update
  PUBLIC sensorring::sensorring
)
