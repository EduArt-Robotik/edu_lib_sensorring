# Compile examples
add_executable(depth_map
    cpp/depth_map/src/main.cpp
    cpp/depth_map/src/MeasurementProxy.cpp
)

target_link_libraries(depth_map
    PRIVATE sensorring::sensorring
)

add_executable(minimal
    cpp/minimal/src/main.cpp
    cpp/minimal/src/MeasurementProxy.cpp
)

target_link_libraries(minimal
    PRIVATE sensorring::sensorring
)

# Install compiled examples
install( TARGETS
    minimal
    depth_map
  RUNTIME DESTINATION ${SENSORRING_INSTALL_EXAMPLES_RUNTIME_DIR} COMPONENT Examples
)

# Example sources
install( DIRECTORY
    cpp
    DESTINATION ${SENSORRING_INSTALL_EXAMPLES_DIR} COMPONENT Examples
)

if(BUILD_PYTHON_BINDINGS)
  install( DIRECTORY
    python3
    DESTINATION ${SENSORRING_INSTALL_EXAMPLES_DIR} COMPONENT Examples
  )
endif()