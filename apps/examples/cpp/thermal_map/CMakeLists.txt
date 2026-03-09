cmake_minimum_required(VERSION 3.13)

project(thermal_map_example)

find_package(sensorring REQUIRED)

add_executable(thermal_map
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(thermal_map
    sensorring::sensorring
)