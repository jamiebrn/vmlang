cmake_minimum_required(VERSION 3.16)
project(assembler LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include_directories(include/)
include_directories(../vm/include/)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(assembler ${SRC_FILES})
target_link_options(assembler PRIVATE -static)
target_compile_features(assembler PRIVATE cxx_std_20)