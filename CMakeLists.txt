cmake_minimum_required(VERSION 3.8)

#########################################################
# build type (has to be done before project() call)
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
ELSE()
   SET(CMAKE_BUILD_TYPE Release)
ENDIF()

#########################################################
# project
set(CMAKE_PROJECT_VERSION_MAJOR 2)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}")

project(sensorring
  LANGUAGES CXX
  VERSION ${CMAKE_PROJECT_VERSION}
  HOMEPAGE_URL "https://github.com/EduArt-Robotik/edu_lib_sensorring"
)

#########################################################
# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles directory.")
endif()

#########################################################
# build settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
  endif()

elseif(MSVC)
  add_compile_options(/Wall)

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(/O2)
  endif()
endif()

#########################################################
# options

if(UNIX)
  option( USE_SOCKETCAN "Add support for Linux SocketCAN" ON)
endif()
option( USE_USBTINGO "Add support for the USBtingo USB to CAN FD converter" OFF)
option( BUILD_SHARED_LIBS "Build as shared library" OFF)
option( VERBOSE_BUILD "Verbose build" OFF)

if(WIN32)
  set( USE_USBTINGO ON)
endif()

if(NOT USE_SOCKETCAN AND NOT USE_USBTINGO)
  message(FATAL_ERROR "At least one of the options USE_SOCKETCAN or USE_USBTINGO has to be turned on!")
endif()

if(USE_USBTINGO)
  find_package(usbtingo REQUIRED)
endif()

find_package(Threads REQUIRED)

#########################################################
# declare library
add_library(sensorring
  src/MeasurementManager.cpp
  src/SensorRing.cpp
  src/SensorBus.cpp
  src/SensorBoard.cpp
  src/interface/ComInterface.cpp
  src/interface/ComManager.cpp
  src/interface/ComObserver.cpp
  src/interface/can/canprotocol.cpp
  src/sensors/TofSensor.cpp
  src/sensors/ThermalSensor.cpp
  src/sensors/LedLight.cpp
  src/sensors/BaseSensor.cpp
  src/utils/Math.cpp
  src/utils/Logger.cpp
  src/utils/CustomTypes.cpp
  src/utils/FileManager.cpp
)

if(USE_SOCKETCAN)
  list(APPEND SOURCES
    src/interface/can/SocketCANFD.cpp
  )
endif()

if(USE_USBTINGO)
  list(APPEND SOURCES
    src/interface/can/USBtingo.cpp
  )
endif()

target_sources(sensorring PRIVATE ${SOURCES})

#########################################################
# library includes
target_include_directories(sensorring PRIVATE
  include/${PROJECT_NAME}
  include/${PROJECT_NAME}/boardmanager
  include/${PROJECT_NAME}/interface
  include/${PROJECT_NAME}/interface/can
  include/${PROJECT_NAME}/math
  include/${PROJECT_NAME}/sensors
  include/${PROJECT_NAME}/types
  include/${PROJECT_NAME}/utils
)

# build macros
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

target_include_directories(sensorring PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# export includes for other packages
target_include_directories(sensorring PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#########################################################
# install library
install(TARGETS sensorring
  EXPORT sensorringTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# install headers
install(FILES
  include/${PROJECT_NAME}/MeasurementManager.hpp
  include/${PROJECT_NAME}/MeasurementObserver.hpp
  include/${PROJECT_NAME}/CustomTypes.hpp
  include/${PROJECT_NAME}/Parameters.hpp
  include/${PROJECT_NAME}/Math.hpp
  DESTINATION include/${PROJECT_NAME}
)

# install cmake files
install(EXPORT sensorringTargets
  FILE sensorringTargets.cmake
  NAMESPACE sensorring::
  DESTINATION lib/cmake/sensorring
)

#########################################################
# cmake package configuration
include(CMakePackageConfigHelpers)

if(USE_USBTINGO)
    set(FIND_DEPENDENCIES "find_dependency(usbtingo REQUIRED)\nfind_dependency(Threads REQUIRED)")
endif()

configure_package_config_file(cmake/Config.cmake.in
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

#########################################################
# get architecture
set(ARCH_TEXT "")
find_program(DPKG_PROGRAM dpkg)
if(EXISTS ${DPKG_PROGRAM})
  execute_process(
    COMMAND dpkg --print-architecture 
    OUTPUT_VARIABLE ARCH_TEXT OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  set(ARCH_TEXT ${CMAKE_SYSTEM_PROCESSOR})
endif()

#########################################################
# cpack packaging

if(CMAKE_BUILD_TYPE MATCHES Release)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME}-${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${SENSORRING_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SENSORRING_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SENSORRING_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${ARCH_TEXT})

set(CPACK_PACKAGE_VENDOR "EduArt Robotik GmbH")
set(CPACK_PACKAGE_CONTACT "Hannes Duske <hannes.duske@eduart-robotik.com>")
set(CPACK_PACKAGE_DESCRIPTION "Software interface of the EduArt Sensorring")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/EduArt-Robotik/edu_lib_sensorring")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_PACKAGE_SECTION "libraries")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.39), libstdc++6 (>= 14.2.0), libgcc-s1 (>=14.2.0)")

set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_OUTPUT_FILE_PREFIX  ../release)

if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
  set(CPACK_GENERATOR "TGZ;DEB")
  set(GENERATORS_TEXT "TGZ;DEB")
else()
  set(CPACK_GENERATOR "ZIP")
  set(GENERATORS_TEXT "ZIP")
endif()

include(CPack)
endif()

#########################################################
# summary of build settings
include(cmake/ShowSummary.cmake REQUIRED)