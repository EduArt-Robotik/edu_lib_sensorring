set(CMAKE_SWIG_FLAGS "")

find_package(SWIG REQUIRED)
include(UseSWIG)

if(SENSORRING_BUILD_PYTHON_BINDINGS)
  add_subdirectory(python)
endif()