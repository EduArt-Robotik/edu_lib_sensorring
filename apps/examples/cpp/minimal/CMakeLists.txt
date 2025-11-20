add_executable(minimal
    src/main.cpp
    src/MeasurementProxy.cpp
)

target_link_libraries(minimal
    sensorring
)