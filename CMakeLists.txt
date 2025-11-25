cmake_minimum_required(VERSION 3.10)

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

#########################################################
# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You can't build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles directory.")
endif()

#########################################################
# Build settings
include(cmake/BuildSettings.cmake REQUIRED)

# options
include(cmake/BuildOptions.cmake REQUIRED)

# Fetch dependencies
include(cmake/CollectDependencies.cmake REQUIRED)

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
    include(cmake/PackageGeneration.cmake REQUIERD)    
endif()

# Build summary
include(cmake/ShowSummary.cmake REQUIRED)