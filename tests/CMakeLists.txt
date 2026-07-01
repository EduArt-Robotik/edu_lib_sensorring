include(CTest)
include(Catch)

add_executable(sensorring_unit_tests
    unit/test_Vector3.cpp
    unit/test_Matrix3.cpp
    unit/test_Math.cpp
    unit/test_PointCloud.cpp
    unit/test_DepthSensor.cpp
    unit/test_DepthMeasurement.cpp
    unit/test_TMF8829_Measurement.cpp
    unit/test_ThermalMeasurement.cpp
    unit/test_ComEndpoint.cpp
    unit/test_DeviceID.cpp
    unit/test_SubscriberToken.cpp
    unit/test_Subscription.cpp
    unit/test_Publisher.cpp
    unit/test_HTPA32_Eeprom.cpp
    unit/test_TMF8829_Params.cpp
    unit/test_SensorRingFactory.cpp
)

target_include_directories(sensorring_unit_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(sensorring_unit_tests PRIVATE
  sensorring::sensorring
  Catch2::Catch2WithMain
  Threads::Threads
)

catch_discover_tests(sensorring_unit_tests
  PROPERTIES LABELS "unit"
)

if(SENSORRING_BUILD_HARDWARE_TESTS)
    add_executable(sensorring_hardware_tests
      hardware/test_DepthSensorHardware.cpp
      hardware/test_TMF8829Hardware.cpp
      hardware/test_SocketCanHardware.cpp
      hardware/test_UsbTingoHardware.cpp
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

