cmake_minimum_required(VERSION 3.13)

project(proxy_example)

find_package(sensorring REQUIRED)

add_executable(proxy
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(proxy
    sensorring::sensorring
)