#########################################################
# install the targets and public headers available in this configuration
include(CMakePackageConfigHelpers)

set(FIND_DEPENDENCIES "find_dependency(Threads REQUIRED)\n")
if(_SENSORRING_TRANSPORT_IMPORTED)
  string(APPEND FIND_DEPENDENCIES "find_dependency(sensorring_transport CONFIG REQUIRED)\n")
endif()
if(SENSORRING_USE_USBTINGO)
  string(APPEND FIND_DEPENDENCIES "find_dependency(usbtingo CONFIG REQUIRED)\n")
endif()
if(SENSORRING_BUILD_FIRMWARE_UPDATE AND NOT SENSORRING_BOOTLOADER_SOURCE_DIR)
  string(APPEND FIND_DEPENDENCIES "find_dependency(frankly_bootloader CONFIG REQUIRED NAMES frankly_bootloader frankly-bootloader)\n")
endif()

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  "${PROJECT_BINARY_DIR}/sensorringConfig.cmake"
  INSTALL_DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/sensorringConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

set(_SENSORRING_INSTALL_TARGETS sensorring_base)
if(SENSORRING_BUILD_LIB)
  list(APPEND _SENSORRING_INSTALL_TARGETS sensorring)
endif()
if(SENSORRING_BUILD_FIRMWARE_UPDATE)
  list(APPEND _SENSORRING_INSTALL_TARGETS sensorring_firmware_update)
endif()
if(NOT _SENSORRING_TRANSPORT_IMPORTED)
  list(APPEND _SENSORRING_INSTALL_TARGETS sensorring_transport)
endif()
install(TARGETS ${_SENSORRING_INSTALL_TARGETS}
  EXPORT sensorringTargets
  LIBRARY DESTINATION ${SENSORRING_INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${SENSORRING_INSTALL_ARCHIVE_DIR}
  RUNTIME DESTINATION ${SENSORRING_INSTALL_RUNTIME_DIR}
)
install(EXPORT sensorringTargets
  FILE sensorringTargets.cmake
  NAMESPACE sensorring::
  DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
)

if(SENSORRING_BUILD_LIB)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}
    PATTERN firmware EXCLUDE
  )
else()
  install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/sensorring/interface
    ${PROJECT_SOURCE_DIR}/include/sensorring/logger
    ${PROJECT_SOURCE_DIR}/include/sensorring/subscription
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring
  )
  install(FILES
    ${PROJECT_SOURCE_DIR}/include/sensorring/board/BoardDiscovery.hpp
    ${PROJECT_SOURCE_DIR}/include/sensorring/board/EnumerationInformation.hpp
    ${PROJECT_SOURCE_DIR}/include/sensorring/board/SensorBoardType.hpp
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring/board
  )
  install(FILES ${PROJECT_SOURCE_DIR}/include/sensorring/device/DeviceType.hpp
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring/device
  )
endif()
if(SENSORRING_BUILD_FIRMWARE_UPDATE)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/sensorring/firmware
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring
  )
endif()
set(_SENSORRING_EXPORT_HEADERS SensorringBaseExport.hpp)
if(SENSORRING_BUILD_LIB)
  list(APPEND _SENSORRING_EXPORT_HEADERS SensorringExport.hpp)
endif()
if(SENSORRING_BUILD_FIRMWARE_UPDATE)
  list(APPEND _SENSORRING_EXPORT_HEADERS SensorringFirmwareUpdateExport.hpp)
endif()
foreach(header IN LISTS _SENSORRING_EXPORT_HEADERS)
  install(FILES ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/${header}
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring/platform
  )
endforeach()

if(NOT _SENSORRING_TRANSPORT_IMPORTED)
  install(DIRECTORY ${SENSORRING_TRANSPORT_SOURCE_DIR}/include/
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}
  )
endif()
install(FILES
  "${PROJECT_BINARY_DIR}/sensorringConfig.cmake"
  "${PROJECT_BINARY_DIR}/sensorringConfigVersion.cmake"
  DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
)
