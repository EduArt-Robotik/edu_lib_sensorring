add_executable(depth_map
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(depth_map
    sensorring
)