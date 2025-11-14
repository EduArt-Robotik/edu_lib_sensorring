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
  sensors/TofSensor.cpp
  sensors/ThermalSensor.cpp
  sensors/LedLight.cpp
  sensors/BaseSensor.cpp
  utils/Math.cpp
  utils/CustomTypes.cpp
  utils/FileManager.cpp
  utils/EnumerationInformation.cpp
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
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

# export includes for other packages
target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
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
  target_compile_definitions(sensorring PRIVATE _USE_MATH_DEFINES)
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
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# install cmake files
install(EXPORT sensorringTargets
  FILE sensorringTargets.cmake
  NAMESPACE sensorring::
  DESTINATION lib/cmake/sensorring
)

# install headers
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
)

#########################################################
# cmake package configuration
include(CMakePackageConfigHelpers)

if(USE_USBTINGO)
    set(FIND_DEPENDENCIES "find_dependency(usbtingo REQUIRED)\nfind_dependency(Threads REQUIRED)")
endif()

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in
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