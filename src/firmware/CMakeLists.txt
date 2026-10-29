#########################################################
# firmware update library

if(SENSORRING_BUILD_SHARED_LIBS)
  add_library(sensorring_firmware_update SHARED)
else()
  add_library(sensorring_firmware_update STATIC)
endif()

add_library(sensorring::firmware_update ALIAS sensorring_firmware_update)
set_target_properties(sensorring_firmware_update PROPERTIES EXPORT_NAME firmware_update)

generate_export_header(sensorring_firmware_update
  EXPORT_FILE_NAME ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/SensorringFirmwareUpdateExport.hpp
)

set(SENSORRING_FIRMWARE_UPDATE_SOURCES
  FirmwareUpdater.cpp
  internal/BootloaderActivator.cpp
  internal/BootloaderDevice.cpp
  internal/BootloaderProtocol.cpp
  internal/Crc32.cpp
  internal/HexFile.cpp
)

if(SENSORRING_BOOTLOADER_SOURCE_DIR)
  list(APPEND SENSORRING_FIRMWARE_UPDATE_SOURCES
    ${SENSORRING_BOOTLOADER_SOURCE_DIR}/src/francor/franklyboot/msg.cpp
  )
  target_include_directories(sensorring_firmware_update PRIVATE
    $<BUILD_INTERFACE:${SENSORRING_BOOTLOADER_SOURCE_DIR}/include>
  )
else()
  target_link_libraries(sensorring_firmware_update PRIVATE
    frankly_bootloader::frankly_bootloader
  )
endif()

target_sources(sensorring_firmware_update PRIVATE
  ${SENSORRING_FIRMWARE_UPDATE_SOURCES}
)

target_include_directories(sensorring_firmware_update
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(sensorring_firmware_update
  PUBLIC sensorring::base
)
