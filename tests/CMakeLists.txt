include(CTest)
include(Catch)

add_executable(sensorring_tests
    unit/test_vector3.cpp
    unit/test_matrix3.cpp
    unit/test_math_utils.cpp
    unit/test_pointcloud.cpp
    unit/test_tof_measurement.cpp
    unit/test_com_endpoints.cpp
    unit/test_can_endpoint_map.cpp
    unit/test_ICapability.cpp
    unit/test_ICapabilityAsync.cpp
    unit/test_IDevice.cpp
)

target_include_directories(sensorring_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(sensorring_tests PRIVATE
  sensorring::sensorring
  Catch2::Catch2WithMain
  Threads::Threads
)

catch_discover_tests(sensorring_tests
  PROPERTIES LABELS "unit"
)

if(SENSORRING_BUILD_HARDWARE_TESTS)
    add_executable(sensorring_hardware_tests
      hardware/test_tof_hardware.cpp
    )

    target_link_libraries(sensorring_hardware_tests PRIVATE
      sensorring::sensorring
      Catch2::Catch2WithMain
      Threads::Threads
    )

    catch_discover_tests(sensorring_hardware_tests
      PROPERTIES LABELS "hardware"
    )
endif()

