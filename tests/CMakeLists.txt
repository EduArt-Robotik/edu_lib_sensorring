include(CTest)
include(Catch)

add_executable(sensorring_tests
    unit/test_vector3.cpp
    unit/test_matrix3.cpp
    unit/test_math_utils.cpp
    unit/test_pointcloud.cpp
    unit/test_tof_measurement.cpp
)

target_link_libraries(sensorring_tests PRIVATE sensorring::sensorring Catch2::Catch2WithMain)
catch_discover_tests(sensorring_tests)

if(SENSORRING_BUILD_HARDWARE_TESTS)
    add_executable(sensorring_hardware_tests hardware/test_tof_hardware.cpp)
    target_link_libraries(sensorring_hardware_tests PRIVATE sensorring::sensorring Catch2::Catch2WithMain)
    catch_discover_tests(sensorring_hardware_tests)
endif()

