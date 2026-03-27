include(CTest)
include(Catch)

add_executable(sensorring_tests
    unit/test_Vector3.cpp
    unit/test_Matrix3.cpp
    unit/test_Math.cpp
    unit/test_PointCloud.cpp
    unit/test_TofMeasurement.cpp
    unit/test_ComEndpoint.cpp
    unit/test_CanEndpointMap.cpp
    unit/test_DeviceID.cpp
    unit/test_DeviceGroup.cpp
    unit/test_SubscriberToken.cpp
    unit/test_Subscription.cpp
    unit/test_Publisher.cpp
    unit/test_MeasurementClient.cpp
    unit/test_HTPA32_Eeprom.cpp
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
      hardware/test_TofHardware.cpp
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

