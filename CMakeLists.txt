cmake_minimum_required(VERSION 3.5)
project(AMOBA)

set(CMAKE_CXX_STANDARD 14)

add_custom_target(
    run ALL COMMAND ${CMAKE_COMMAND}
)
add_executable(AMOBA src/main.cpp)