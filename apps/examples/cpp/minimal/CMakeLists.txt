add_executable(minimal
    main.cpp
    MeasurementProxy.cpp
)

target_link_libraries(minimal
    sensorring
)