# Compile examples

add_executable(minimal
    cpp/minimal/src/main.cpp
    cpp/minimal/src/Rate.cpp
)

target_link_libraries(minimal
    PRIVATE sensorring::sensorring
)

add_executable(depth_map
    cpp/depth_map/src/main.cpp
)

target_link_libraries(depth_map
    PRIVATE sensorring::sensorring
)

add_executable(thermal_map
    cpp/thermal_map/src/main.cpp
)

target_link_libraries(thermal_map
    PRIVATE sensorring::sensorring
)

add_executable(extra_action
    cpp/extra_action/src/main.cpp
)

target_link_libraries(extra_action
    PRIVATE sensorring::sensorring
)

add_executable(proxy_class
    cpp/proxy_class/src/main.cpp
    cpp/proxy_class/src/CustomProxy.cpp
    cpp/proxy_class/src/Rate.cpp
)

target_link_libraries(proxy_class
    PRIVATE sensorring::sensorring
)

add_executable(client_interface
    cpp/client_interface/src/main.cpp
    cpp/client_interface/src/CustomClient.cpp
    cpp/client_interface/src/Rate.cpp
)

target_link_libraries(client_interface
    PRIVATE sensorring::sensorring
)

if(SENSORRING_INSTALL)
  # Install compiled examples
  install( TARGETS
    minimal
    proxy_class
    client_interface
    depth_map
    thermal_map
    extra_action
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
endif()