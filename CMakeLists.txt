# cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address  -fsanitize=leak -g" -DCMAKE_C_FLAGS="-fsanitize=address  -fsanitize=leak -g" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address  -fsanitize=leak" -DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=address  -fsanitize=leak"

cmake_minimum_required(VERSION 2.5)
project(AMOBA)

set(CMAKE_CXX_STANDARD 14)

file(GLOB AMOBA_FILES src/*.cpp src/*.hpp)
add_library(amoba ${AMOBA_FILES})

file(GLOB NAUTY_FILES nauty27r1/nauty.c nauty27r1/nautil.c nauty27r1/naugraph.c nauty27r1/schreier.c nauty27r1/naurng.c)
add_library(nauty ${NAUTY_FILES})

#message("${FILES}")
include_directories(src)

set(CMAKE_CXX_FLAGS "-O3")
add_executable(AMOBA test/main.cpp)

add_executable(AMOBA_DEBUG test/main.cpp)
#set_target_properties(AMOBA_DEBUG PROPERTIES COMPILE_FLAGS "-Wall -Wno-long-long -pedantic")
set_target_properties(AMOBA_DEBUG PROPERTIES COMPILE_FLAGS "-g")

add_executable(TEST test/test.cpp)
set_target_properties(TEST PROPERTIES COMPILE_FLAGS "-g")

add_executable(NAUTY test/nauty.cpp)
set_target_properties(NAUTY PROPERTIES COMPILE_FLAGS "-g")

file(COPY ${CMAKE_BINARY_DIR}/../test/measure.sh  ${CMAKE_BINARY_DIR}/../test/PLAY
  DESTINATION ${CMAKE_BINARY_DIR}
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

target_link_libraries(AMOBA amoba nauty)
target_link_libraries(AMOBA_DEBUG amoba nauty)
target_link_libraries(TEST amoba nauty)
target_link_libraries(NAUTY amoba nauty)
