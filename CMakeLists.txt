cmake_minimum_required(VERSION 3.13)

cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

#########################################################
# Build type (has to be done before project() call)
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug/ Release/ RelWithDebInfo)" FORCE)
endif()

#########################################################
# Project
set(CMAKE_PROJECT_VERSION_MAJOR 2)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}")

project(sensorring
  LANGUAGES CXX
  VERSION ${CMAKE_PROJECT_VERSION}
  HOMEPAGE_URL "https://github.com/EduArt-Robotik/edu_lib_sensorring"
)

set(SENSORRING_NAME               "Sensor Ring Library")
set(SENSORRING_COMPANY            "EduArt Robotik GmbH")
set(SENSORRING_CONTACT            "Hannes Duske <hannes.duske@eduart-robotik.com>")
set(SENSORRING_DESCRIPTION        "Software interface of the EduArt Sensor Ring")
set(SENSORRING_VERSION            ${PROJECT_VERSION})
set(SENSORRING_ABBREVIATION       ${PROJECT_NAME})
set(SENSORRING_HOMEPAGE_URL       ${PROJECT_HOMEPAGE_URL})


#########################################################
# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You can't build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles directory.")
endif()

#########################################################
# For includes and find modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Configure Platform
include(ConfigurePlatform REQUIRED)

# Configure Paths
include(ConfigurePaths REQUIRED)

# Build settings
include(BuildSettings REQUIRED)

# options
include(BuildOptions REQUIRED)

# Fetch dependencies
include(CollectDependencies REQUIRED)

# Library
add_subdirectory(src)

# Examples
if(BUILD_EXAMPLES)
  add_subdirectory(apps/examples/cpp)
endif()

# Documentation
if(BUILD_DOCUMENTATION AND CMAKE_BUILD_TYPE MATCHES Release)
  add_subdirectory(doc)
endif()

# Language bindings
if(BUILD_PYTHON_BINDINGS AND CMAKE_BUILD_TYPE MATCHES Release)
  add_subdirectory(bindings)
endif()

# cpack packaging
if(CMAKE_BUILD_TYPE MATCHES Release)
    include(PackageGeneration REQUIERD)    
endif()

# Build summary
include(ShowSummary REQUIRED)