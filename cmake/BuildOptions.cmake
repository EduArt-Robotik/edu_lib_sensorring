#########################################################
# options

if(IS_LINUX)
  option( SENSORRING_USE_SOCKETCAN "Compile with support for Linux SocketCAN" ON)
endif()
option( SENSORRING_USE_USBTINGO "Compile with support for the USBtingo USB adapter" OFF)
option( SENSORRING_INSTALL "Enable the installation of the library." on)
option( SENSORRING_BUILD_SHARED_LIBS "Build as shared library. If set to OFF a static library is built." OFF)
option( SENSORRING_BUILD_EXAMPLES "Build the example programs" OFF)
option( SENSORRING_BUILD_UTILS "Build utility command line programs" OFF)
option( SENSORRING_BUILD_DOCUMENTATION "Build the documentation" OFF)
option( SENSORRING_BUILD_PYTHON_BINDINGS "Build python bindings" OFF)
option( SENSORRING_BUILD_TESTS "Build unit tests" OFF)
option( SENSORRING_BUILD_HARDWARE_TESTS "Build hardware-dependent tests" OFF)

if(IS_WINDOWS)
  set( SENSORRING_USE_USBTINGO ON)
endif()

if(NOT SENSORRING_USE_SOCKETCAN AND NOT SENSORRING_USE_USBTINGO)
  message(FATAL_ERROR "At least one of the options USE_SOCKETCAN or USE_USBTINGO has to be turned on!")
endif()

# clangd compile_commands.json support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)