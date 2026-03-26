# Compile examples

add_executable(minimal_using_lambdas
    cpp/minimal/using_lambdas.cpp
)

target_link_libraries(minimal_using_lambdas
    PRIVATE sensorring::sensorring
)

add_executable(minimal_using_proxy_class
    cpp/minimal/using_proxy_class.cpp
)

target_link_libraries(minimal_using_proxy_class
    PRIVATE sensorring::sensorring
)

add_executable(minimal_using_client_interface
    cpp/minimal/using_client_interface.cpp
)

target_link_libraries(minimal_using_client_interface
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

if(SENSORRING_INSTALL)
  # Install compiled examples
  install( TARGETS
    minimal_using_lambdas
    minimal_using_proxy_class
    minimal_using_client_interface
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