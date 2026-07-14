add_executable(discovery_tool
    src/main.cpp
)

target_link_libraries(discovery_tool
    PRIVATE
      sensorring::sensorring
)
