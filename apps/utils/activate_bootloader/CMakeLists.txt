add_executable(activate_bootloader
    src/main.cpp
)

target_link_libraries(activate_bootloader
    PRIVATE
      sensorring::sensorring
)
