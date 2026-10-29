#########################################################
# communication and board discovery, independent of sensor drivers
if(SENSORRING_BUILD_SHARED_LIBS)
  add_library(sensorring_base SHARED)
else()
  add_library(sensorring_base STATIC)
endif()

add_library(sensorring::base ALIAS sensorring_base)
set_target_properties(sensorring_base PROPERTIES EXPORT_NAME base)

target_sources(sensorring_base PRIVATE
  ../interface/ComInterfaceID.cpp
  ../interface/ComInterface.cpp
  ../interface/ComManager.cpp
  ../interface/ComEndpoint.cpp
  ../interface/can/CanInterface.cpp
  ../logger/Logger.cpp
  ../logger/LogVerbosity.cpp
  ../board/BoardDiscovery.cpp
  ../board/BoardEnumerator.cpp
  ../board/EnumerationInformation.cpp
  ../board/SensorBoardType.cpp
  ../device/DeviceType.cpp
)

if(SENSORRING_USE_SOCKETCAN)
  target_sources(sensorring_base PRIVATE ../interface/can/socketcan/SocketCANFD.cpp)
  target_compile_definitions(sensorring_base PRIVATE USE_SOCKETCAN)
endif()

if(SENSORRING_USE_USBTINGO)
  target_sources(sensorring_base PRIVATE ../interface/can/usbtingo/USBtingo.cpp)
  target_compile_definitions(sensorring_base PRIVATE USE_USBTINGO)
  target_link_libraries(sensorring_base PRIVATE usbtingo::usbtingo)
endif()

target_link_libraries(sensorring_base
  PUBLIC sensorring_transport::sensorring_transport Threads::Threads
)

target_include_directories(sensorring_base
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SENSORRING_GENERATED_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

generate_export_header(sensorring_base
  EXPORT_FILE_NAME ${SENSORRING_GENERATED_INCLUDE_DIR}/sensorring/platform/SensorringBaseExport.hpp
)
