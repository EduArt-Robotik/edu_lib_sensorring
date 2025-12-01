#########################################################
# declare library
add_library(sensorring
  MeasurementManager.cpp
  MeasurementClient.cpp
  MeasurementManagerImpl.cpp
  SensorRing.cpp
  SensorBus.cpp
  SensorBoard.cpp
  interface/ComInterface.cpp
  interface/ComManager.cpp
  interface/ComObserver.cpp
  interface/can/canprotocol.cpp
  logger/Logger.cpp
  logger/LoggerClient.cpp
  sensors/TofSensor.cpp
  sensors/ThermalSensor.cpp
  sensors/LedLight.cpp
  sensors/BaseSensor.cpp
  types/Image.cpp
  types/PointCloud.cpp
  types/EnumerationInformation.cpp
  utils/FileManager.cpp
  math/Math.cpp
  math/Vector3.cpp
  math/Matrix3.cpp
)

if(USE_SOCKETCAN)
  list(APPEND SOURCES
    interface/can/SocketCANFD.cpp
  )
endif()

if(USE_USBTINGO)
  list(APPEND SOURCES
    interface/can/USBtingo.cpp
  )
endif()

target_sources(sensorring PRIVATE ${SOURCES})

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
if(BUILD_SHARED_LIBS)
  target_compile_definitions(sensorring PUBLIC
    SENSORRING_SHARED
  )
endif()

if(MSVC)
  target_compile_definitions(sensorring PRIVATE
  _USE_MATH_DEFINES
  SENSORRING_DLL_EXPORT)
endif()

if(USE_SOCKETCAN)
  target_compile_definitions(sensorring PRIVATE USE_SOCKETCAN)
endif()

if(USE_USBTINGO)
  target_compile_definitions(sensorring PRIVATE USE_USBTINGO)
  if(BUILD_SHARED_LIBS)
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
# install library
install(TARGETS sensorring
  EXPORT sensorringTargets
  LIBRARY DESTINATION ${SENSORRING_INSTALL_LIBRARY_DIR}
  ARCHIVE DESTINATION ${SENSORRING_INSTALL_ARCHIVE_DIR}
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

#########################################################
# cmake package configuration
include(CMakePackageConfigHelpers)

if(USE_USBTINGO)
    set(FIND_DEPENDENCIES "find_dependency(usbtingo REQUIRED)\nfind_dependency(Threads REQUIRED)")
endif()

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
  INSTALL_DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
)

write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
  VERSION "${sensorringVERSION}"
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_BINARY_DIR}/sensorringConfig.cmake"
  "${CMAKE_BINARY_DIR}/sensorringConfigVersion.cmake"
  DESTINATION ${SENSORRING_INSTALL_CMAKE_DIR}
)