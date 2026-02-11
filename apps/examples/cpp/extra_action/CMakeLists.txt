cmake_minimum_required(VERSION 3.13)

project(extra_action_example)

find_package(sensorring REQUIRED)

add_executable(extra_action_example
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(extra_action_example
    sensorring::sensorring
)