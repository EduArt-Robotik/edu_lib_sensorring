add_executable(firmware_updater
    src/main.cpp
)

target_link_libraries(firmware_updater
    PRIVATE
      sensorring::sensorring
      sensorring::firmware_update
)
