#########################################################
# firmware update library

if(SENSORRING_BUILD_SHARED_LIBS)
  add_library(sensorring_firmware_update SHARED)
else()
  add_library(sensorring_firmware_update STATIC)
endif()

add_library(sensorring::firmware_update ALIAS sensorring_firmware_update)

set(SENSORRING_FIRMWARE_UPDATE_SOURCES
  FirmwareUpdater.cpp
  internal/BootloaderActivator.cpp
  internal/BootloaderDevice.cpp
  internal/BootloaderProtocol.cpp
  internal/Crc32.cpp
  internal/HexFile.cpp
  ${SENSORRING_BOOTLOADER_SOURCE_DIR}/src/francor/franklyboot/msg.cpp
)

target_sources(sensorring_firmware_update PRIVATE
  ${SENSORRING_FIRMWARE_UPDATE_SOURCES}
)

target_include_directories(sensorring_firmware_update
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${SENSORRING_BOOTLOADER_SOURCE_DIR}/include>
)

target_link_libraries(sensorring_firmware_update
  PUBLIC sensorring::sensorring
)
