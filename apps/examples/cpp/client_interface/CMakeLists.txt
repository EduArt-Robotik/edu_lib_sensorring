cmake_minimum_required(VERSION 3.13)

project(client_interface_example)

find_package(sensorring REQUIRED)

add_executable(client_interface
    src/main.cpp
    src/CustomClient.cpp
    src/Rate.cpp
)

target_link_libraries(client_interface
    sensorring::sensorring
)