add_executable(discovery_tool
    src/main.cpp
)

target_link_libraries(discovery_tool
    PRIVATE
      sensorring::base
)

if(SENSORRING_INSTALL)
  install(TARGETS discovery_tool RUNTIME DESTINATION ${SENSORRING_INSTALL_RUNTIME_DIR})
endif()
