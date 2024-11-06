cmake_minimum_required(VERSION 3.8)
project(sensorring LANGUAGES CXX VERSION 0.1.0)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # add_compile_options(-Wall -Wextra -Wpedantic -Os) # build with optimization
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_library(sensorring SHARED
  src/MeasurementManager.cpp
  src/SensorRing.cpp
  src/SensorBus.cpp
  src/SensorBoard.cpp
  src/sensors/TofSensor.cpp
  src/sensors/ThermalSensor.cpp
  src/sensors/LedLight.cpp
  src/sensors/BaseSensor.cpp
  src/interface/ComInterface.cpp
  src/interface/ComManager.cpp
  src/interface/ComObserver.cpp
  src/interface/can/canprotocol.cpp
  src/interface/can/SocketCANFD.cpp
  src/utils/CustomTypes.cpp
  src/utils/FileManager.cpp
  src/math/Math.cpp
)

target_include_directories( sensorring PRIVATE
  include/${PROJECT_NAME}
  include/${PROJECT_NAME}/interface
  include/${PROJECT_NAME}/interface/can
  include/${PROJECT_NAME}/math
  include/${PROJECT_NAME}/utils
  include/${PROJECT_NAME}/sensors
  include/${PROJECT_NAME}/boardmanager
)

target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS sensorring
  EXPORT sensorringTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES
  include/${PROJECT_NAME}/MeasurementManager.hpp
  include/${PROJECT_NAME}/MeasurementObservers.hpp
  include/${PROJECT_NAME}/utils/CustomTypes.hpp
  include/${PROJECT_NAME}/utils/Parameters.hpp
  include/${PROJECT_NAME}/math/Math.hpp
  DESTINATION include/${PROJECT_NAME}
)

install(EXPORT sensorringTargets
  FILE sensorringTargets.cmake
  NAMESPACE sensorring::
  DESTINATION lib/cmake/sensorring
)

#---------------------------------------------------------------
#--- Package configuration
#---------------------------------------------------------------

include(CMakePackageConfigHelpers)

configure_package_config_file(Config.cmake.in
        "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
        INSTALL_DESTINATION lib/cmake/sensorring
)

write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
        VERSION "${sensorringVERSION}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
        "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
        DESTINATION lib/cmake/sensorring
)