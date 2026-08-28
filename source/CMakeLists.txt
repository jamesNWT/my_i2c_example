cmake_minimum_required(VERSION 3.13...4.42)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(pico_sdk_import.cmake)

project(my_i2c_example ASM)

pico_sdk_init()

# add_executable(i2c i2c.c)
