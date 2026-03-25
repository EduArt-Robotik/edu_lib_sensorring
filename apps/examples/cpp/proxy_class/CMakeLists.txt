cmake_minimum_required(VERSION 3.13)

project(proxy_class_example)

find_package(sensorring REQUIRED)

add_executable(proxy_class
    src/main.cpp
    src/CustomProxy.cpp
    src/Rate.cpp
)

target_link_libraries(proxy_class
    sensorring::sensorring
)