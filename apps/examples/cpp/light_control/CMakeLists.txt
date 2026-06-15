cmake_minimum_required(VERSION 3.13)

project(light_control_example)

find_package(sensorring REQUIRED)

add_executable(light_control
    src/main.cpp
)

target_link_libraries(light_control
    sensorring::sensorring
)