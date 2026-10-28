#########################################################
# Transport module
find_package(sensorring_transport MODULE REQUIRED)

#########################################################
# Frankly bootloader
if(SENSORRING_BUILD_FIRMWARE_UPDATE)
  find_package(frankly_bootloader MODULE REQUIRED)
endif()

#########################################################
# USBtingo (optional dependency)
if(SENSORRING_USE_USBTINGO)
  find_package(usbtingo MODULE REQUIRED)
endif()

#########################################################
# Catch2 (optional dependency for tests)
if(SENSORRING_BUILD_TESTS)
  find_package(Catch2 MODULE REQUIRED)
endif()

find_package(Threads REQUIRED)