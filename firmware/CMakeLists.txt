cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(water_leakage_detection C CXX ASM)
pico_sdk_init()

add_executable(water_leakage_detection
    main.c
    sensors.c
    valve.c
    alerts.c
)

target_link_libraries(water_leakage_detection pico_stdlib hardware_gpio hardware_i2c)

pico_add_extra_outputs(water_leakage_detection)