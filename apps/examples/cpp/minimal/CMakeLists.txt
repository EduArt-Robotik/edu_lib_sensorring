cmake_minimum_required(VERSION 3.13)

project(minimal_examples)

find_package(sensorring REQUIRED)

add_executable(minimal_using_lambdas
    using_lambdas.cpp
)

target_link_libraries(minimal_using_lambdas
    sensorring::sensorring
)

add_executable(minimal_using_proxy_class
    using_proxy_class.cpp
)

target_link_libraries(minimal_using_proxy_class
    sensorring::sensorring
)

add_executable(minimal_using_client_interface
    using_client_interface.cpp
)

target_link_libraries(minimal_using_client_interface
    sensorring::sensorring
)
