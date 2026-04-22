# Compile examples

add_executable(minimal_using_lambdas
    cpp/minimal/src/using_lambdas.cpp
)

target_include_directories(minimal_using_lambdas
    PRIVATE cpp/minimal/include
)

target_link_libraries(minimal_using_lambdas
    PRIVATE sensorring::sensorring
)

add_executable(minimal_using_proxy_class
    cpp/minimal/src/using_proxy_class.cpp
)

target_include_directories(minimal_using_proxy_class
    PRIVATE cpp/minimal/include
)

target_link_libraries(minimal_using_proxy_class
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

add_executable(expert
    cpp/expert/src/main.cpp
)

target_link_libraries(expert
    PRIVATE sensorring::sensorring
)

if(SENSORRING_INSTALL)
  # Install compiled examples
  install( TARGETS
    minimal_using_lambdas
    minimal_using_proxy_class
    depth_map
    thermal_map
    extra_action
    expert
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