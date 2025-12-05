cmake_minimum_required(VERSION 3.13)

project(depth_map_example)

add_executable(depth_map
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(depth_map
    sensorring::sensorring
)