#########################################################
# main sensor ring library
if(SENSORRING_BUILD_SHARED_LIBS)
    add_library(sensorring SHARED)
else()
    add_library(sensorring STATIC)
endif()

add_library(sensorring::sensorring ALIAS sensorring)

set(SENSORRING_GENERATED_INCLUDE_DIR
  ${CMAKE_CURRENT_BINARY_DIR}/generated
)

set(SENSORRING_CORE_SOURCES
  SensorBus.cpp
  SensorRing.cpp
  SensorRingFactory.cpp
)

set(SENSORRING_MANAGER_SOURCES
  manager/ManagerState.cpp
  manager/MeasurementManager.cpp
  manager/MeasurementManagerImpl.cpp
)

set(SENSORRING_INTERFACE_SOURCES
  interface/ComInterfaceID.cpp
  interface/ComInterface.cpp
  interface/ComManager.cpp
  interface/ComEndpoint.cpp
)

if(SENSORRING_USE_SOCKETCAN)
  list(APPEND SENSORRING_INTERFACE_SOURCES
    interface/can/SocketCANFD.cpp
  )
endif()

if(SENSORRING_USE_USBTINGO)
  list(APPEND SENSORRING_INTERFACE_SOURCES
    interface/can/USBtingo.cpp
  )
endif()

set(SENSORRING_LOGGER_SOURCES
  logger/Logger.cpp
  logger/LogVerbosity.cpp
)

set(SENSORRING_BOARD_SOURCES
  board/SensorBoard.cpp
  board/SensorBoardType.cpp
  board/SensorBoardCommands.cpp
  board/SensorBoardManager.cpp
  board/BoardEnumerator.cpp
  board/EnumerationInformation.cpp
)

set(SENSORRING_DEVICE_SOURCES
  device/BaseDevice.cpp
  device/DeviceType.cpp
  device/types/DeviceState.cpp
  device/depth/DepthSensor.cpp
  device/depth/vl53l8cx/VL53L8CX_Device.cpp
  device/depth/vl53l8cx/VL53L8CX_DeviceImpl.cpp
  device/depth/tmf8829/TMF8829_Device.cpp
  device/depth/tmf8829/TMF8829_DeviceImpl.cpp
  device/depth/tmf8829/TMF8829_Measurement.cpp
  device/thermal/htpa32/htpa32_eeprom.c
  device/thermal/htpa32/HTPA32_Device.cpp
  device/thermal/htpa32/HTPA32_DeviceImpl.cpp
  device/light/ws2812b/WS2812b_Device.cpp
)

set(SENSORRING_MEASUREMENT_SOURCES
  measurement/Image.cpp
  measurement/PointCloud.cpp
  measurement/ThermalMeasurement.cpp
)

set(SENSORRING_UTILS_SOURCES
  utils/FileManager.cpp
)

set(SENSORRING_MATH_SOURCES
  math/Math.cpp
  math/Vector3.cpp
  math/Matrix3.cpp
)

add_library(sensorring_core OBJECT ${SENSORRING_CORE_SOURCES})
add_library(sensorring_manager OBJECT ${SENSORRING_MANAGER_SOURCES})
add_library(sensorring_interface OBJECT ${SENSORRING_INTERFACE_SOURCES})
add_library(sensorring_logger OBJECT ${SENSORRING_LOGGER_SOURCES})
add_library(sensorring_board OBJECT ${SENSORRING_BOARD_SOURCES})
add_library(sensorring_device OBJECT ${SENSORRING_DEVICE_SOURCES})
add_library(sensorring_measurement OBJECT ${SENSORRING_MEASUREMENT_SOURCES})
add_library(sensorring_utils OBJECT ${SENSORRING_UTILS_SOURCES})
add_library(sensorring_math OBJECT ${SENSORRING_MATH_SOURCES})

set(SENSORRING_OBJECT_LIBS
  sensorring_core
  sensorring_manager
  sensorring_interface
  sensorring_logger
  sensorring_board
  sensorring_device
  sensorring_measurement
  sensorring_utils
  sensorring_math
)

foreach(objlib IN LISTS SENSORRING_OBJECT_LIBS)
  if(SENSORRING_BUILD_SHARED_LIBS)
    set_target_properties(${objlib} PROPERTIES
      POSITION_INDEPENDENT_CODE ON
    )
  endif()

  target_include_directories(${objlib} PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SENSORRING_GENERATED_INCLUDE_DIR}>
  )

  target_link_libraries(${objlib} PRIVATE
    sensorring_transport::sensorring_transport
  )

  if(SENSORRING_USE_USBTINGO)
    target_link_libraries(${objlib} PRIVATE
      usbtingo::usbtingo
    )
  endif()

  if(MSVC)
    target_compile_definitions(${objlib} PRIVATE
      USE_MATH_DEFINES
    )
  endif()

  if(SENSORRING_USE_SOCKETCAN)
    target_compile_definitions(${objlib} PRIVATE
      USE_SOCKETCAN
    )
  endif()

  if(SENSORRING_USE_USBTINGO)
    target_compile_definitions(${objlib} PRIVATE
      USE_USBTINGO
    )
  endif()
endforeach()

target_sources(sensorring PRIVATE
  $<TARGET_OBJECTS:sensorring_core>
  $<TARGET_OBJECTS:sensorring_manager>
  $<TARGET_OBJECTS:sensorring_interface>
  $<TARGET_OBJECTS:sensorring_logger>
  $<TARGET_OBJECTS:sensorring_board>
  $<TARGET_OBJECTS:sensorring_device>
  $<TARGET_OBJECTS:sensorring_measurement>
  $<TARGET_OBJECTS:sensorring_utils>
  $<TARGET_OBJECTS:sensorring_math>
)


#########################################################
# generate export header
include(GenerateExportHeader)

generate_export_header(sensorring
  EXPORT_FILE_NAME ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/SensorringExport.hpp
)

target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${SENSORRING_GENERATED_INCLUDE_DIR}>
)

install(FILES
  ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/SensorringExport.hpp
  DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}/sensorring/platform
)


#########################################################
# library includes
target_include_directories(sensorring PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

# export includes for other packages
target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)


#########################################################
# build macros
if(MSVC)
  target_compile_definitions(sensorring PRIVATE
    USE_MATH_DEFINES
  )
endif()

if(SENSORRING_USE_SOCKETCAN)
  target_compile_definitions(sensorring PRIVATE
    USE_SOCKETCAN
  )
endif()

#########################################################
# link transport module (always required)
target_link_libraries(sensorring PUBLIC
  sensorring_transport::sensorring_transport
)

#########################################################
# link usbtingo
if(SENSORRING_USE_USBTINGO)
  target_compile_definitions(sensorring PRIVATE USE_USBTINGO)
  if(SENSORRING_BUILD_SHARED_LIBS)
    target_link_libraries(sensorring PRIVATE
      usbtingo::usbtingo
      Threads::Threads
    )
  else()
    target_link_libraries(sensorring PUBLIC
      usbtingo::usbtingo
      Threads::Threads
    )
  endif()
endif()


#########################################################
# cmake package configuration
if(SENSORRING_INSTALL)

  include(CMakePackageConfigHelpers)

  if(SENSORRING_USE_USBTINGO)
      set(FIND_DEPENDENCIES "find_dependency(usbtingo REQUIRED)\nfind_dependency(Threads REQUIRED)")
  endif()

  configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
    INSTALL_DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
  )

  write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
    VERSION "${sensorringVERSION}"
    COMPATIBILITY SameMajorVersion
  )

  #########################################################
  # install library
  install(TARGETS sensorring sensorring_transport
    EXPORT sensorringTargets
    LIBRARY DESTINATION ${SENSORRING_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${SENSORRING_INSTALL_ARCHIVE_DIR}
    RUNTIME DESTINATION ${SENSORRING_INSTALL_RUNTIME_DIR}
  )

  # install cmake files
  install(EXPORT sensorringTargets
    FILE sensorringTargets.cmake
    NAMESPACE sensorring::
    DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
  )

  # install headers
  install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}
  )

  install(DIRECTORY
    ${SENSORRING_TRANSPORT_SOURCE_DIR}/include/
    DESTINATION ${SENSORRING_INSTALL_INCLUDE_DIR}
  )

  install(FILES
    "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
    "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
    DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
  )
endif()