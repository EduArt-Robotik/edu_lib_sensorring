#########################################################
# shared base for the library and utilities
add_subdirectory(base)

if(NOT SENSORRING_BUILD_LIB)
  return()
endif()

#########################################################
# main sensor ring library
if(SENSORRING_BUILD_SHARED_LIBS)
    add_library(sensorring SHARED)
else()
    add_library(sensorring STATIC)
endif()

add_library(sensorring::sensorring ALIAS sensorring)

set(SENSORRING_CORE_SOURCES
  SensorBus.cpp
  SensorRing.cpp
  SensorRingFactory.cpp
  factory/SensorRingFactoryImpl.cpp
)

set(SENSORRING_MANAGER_SOURCES
  manager/ManagerState.cpp
  manager/MeasurementManager.cpp
  manager/MeasurementManagerImpl.cpp
)

set(SENSORRING_BOARD_SOURCES
  board/SensorBoard.cpp
  board/SensorBoardParams.cpp
  board/SensorBoardManager.cpp
)

set(SENSORRING_DEVICE_SOURCES
  device/Device.cpp
  device/Sensor.cpp
  device/types/DeviceState.cpp
  device/depth/DepthSensor.cpp
  device/thermal/ThermalSensor.cpp
  device/depth/vl53l8cx/VL53L8CX_Device.cpp
  device/depth/vl53l8cx/VL53L8CX_DeviceImpl.cpp
  device/depth/tmf8829/TMF8829_Device.cpp
  device/depth/tmf8829/TMF8829_Params.cpp
  device/depth/tmf8829/TMF8829_DeviceImpl.cpp
  device/depth/tmf8829/TMF8829_Measurement.cpp
  device/depth/tmf8829/TMF8829_ResultFormat.cpp
  device/thermal/htpa32/htpa32_eeprom.c
  device/thermal/htpa32/HTPA32_Device.cpp
  device/thermal/htpa32/HTPA32_DeviceImpl.cpp
  device/light/Light.cpp
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
  math/Pose.cpp  
  math/Math.cpp
  math/Vector3.cpp
  math/Matrix3.cpp
)

add_library(sensorring_core OBJECT ${SENSORRING_CORE_SOURCES})
add_library(sensorring_manager OBJECT ${SENSORRING_MANAGER_SOURCES})
add_library(sensorring_board OBJECT ${SENSORRING_BOARD_SOURCES})
add_library(sensorring_device OBJECT ${SENSORRING_DEVICE_SOURCES})
add_library(sensorring_measurement OBJECT ${SENSORRING_MEASUREMENT_SOURCES})
add_library(sensorring_utils OBJECT ${SENSORRING_UTILS_SOURCES})
add_library(sensorring_math OBJECT ${SENSORRING_MATH_SOURCES})

set(SENSORRING_OBJECT_LIBS
  sensorring_core
  sensorring_manager
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
    sensorring::base
  )

  if(SENSORRING_BUILD_SHARED_LIBS)
    target_compile_definitions(${objlib} PRIVATE sensorring_EXPORTS)
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
  $<TARGET_OBJECTS:sensorring_board>
  $<TARGET_OBJECTS:sensorring_device>
  $<TARGET_OBJECTS:sensorring_measurement>
  $<TARGET_OBJECTS:sensorring_utils>
  $<TARGET_OBJECTS:sensorring_math>
)


#########################################################
# generate export header
generate_export_header(sensorring
  EXPORT_FILE_NAME ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/SensorringExport.hpp
)

target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${SENSORRING_GENERATED_INCLUDE_DIR}>
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
# link shared base
target_link_libraries(sensorring PUBLIC
  sensorring::base
)