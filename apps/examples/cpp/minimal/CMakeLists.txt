cmake_minimum_required(VERSION 3.13)

project(minimal_examples)

find_package(sensorring REQUIRED)

add_executable(minimal_using_lambdas
    src/using_lambdas.cpp
)

target_include_directories(minimal_using_lambdas
    PRIVATE include
)

target_link_libraries(minimal_using_lambdas
    sensorring::sensorring
)

add_executable(minimal_using_proxy_class
    src/using_proxy_class.cpp
)

target_include_directories(minimal_using_proxy_class
    PRIVATE include
)

target_link_libraries(minimal_using_proxy_class
    sensorring::sensorring
)
