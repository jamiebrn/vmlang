cmake_minimum_required(VERSION 3.16)
project(compiler LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include/)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(compiler ${SRC_FILES})
target_link_options(compiler PRIVATE -static)
target_compile_features(compiler PRIVATE cxx_std_20)