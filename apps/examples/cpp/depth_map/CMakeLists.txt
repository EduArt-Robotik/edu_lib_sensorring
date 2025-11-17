add_executable(depth_map
    main.cpp
    MeasurementProxy.cpp
)

target_link_libraries(depth_map
    sensorring
)