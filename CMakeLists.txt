# cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address  -fsanitize=leak -g" -DCMAKE_C_FLAGS="-fsanitize=address  -fsanitize=leak -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address  -fsanitize=leak" -DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=address  -fsanitize=leak"

cmake_minimum_required(VERSION 3.5)
project(AMOBA)

set(CMAKE_CXX_STANDARD 14)

file(GLOB FILES src/*.cpp src/*.hpp)
add_library(mylib ${FILES})

include_directories(src)

set(CMAKE_CXX_FLAGS "-O3")
add_executable(AMOBA src/main.cpp)

set(CMAKE_CXX_FLAGS "-g")
add_executable(AMOBA_DEBUG src/main.cpp)

set(CMAKE_CXX_FLAGS "-g")
add_executable(TEST test/test.cpp)

target_link_libraries(AMOBA mylib)
target_link_libraries(AMOBA_DEBUG mylib)
target_link_libraries(TEST mylib)
