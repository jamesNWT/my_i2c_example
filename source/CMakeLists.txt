cmake_minimum_required(VERSION 3.13...4.42)

include(pico_sdk_import.cmake)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(my_i2c_example C CXX ASM)

pico_sdk_init()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(i2c i2c.c)

target_link_libraries(i2c pico_stdlib)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(i2c 0)
pico_enable_stdio_usb(i2c 1)

pico_add_extra_outputs(i2c)

