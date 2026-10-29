add_executable(firmware_updater
    src/main.cpp
)

target_link_libraries(firmware_updater
    PRIVATE
      sensorring::firmware_update
)

if(SENSORRING_INSTALL)
  install(TARGETS firmware_updater RUNTIME DESTINATION ${SENSORRING_INSTALL_RUNTIME_DIR})
endif()
