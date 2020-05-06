cmake_minimum_required(VERSION 3.5)
project(AMOBA)

set(CMAKE_CXX_STANDARD 17)

add_custom_target(
    run ALL COMMAND ${CMAKE_COMMAND}
)
set(CMAKE_CXX_FLAGS "-O3")
add_executable(AMOBA src/main.cpp src/common.cpp src/heuristic.cpp src/MCTS.cpp src/tree.cpp)
set(CMAKE_CXX_FLAGS "-g")
add_executable(AMOBA_DEBUG src/main.cpp src/common.cpp src/heuristic.cpp src/MCTS.cpp src/tree.cpp)