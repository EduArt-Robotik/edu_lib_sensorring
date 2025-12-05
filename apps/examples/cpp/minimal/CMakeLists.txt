cmake_minimum_required(VERSION 3.13)

project(minimal_example)

add_executable(minimal
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(minimal
    sensorring::sensorring
)