cmake_minimum_required(VERSION 3.16)
project(virtualmachine LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.32.4
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL2)

include_directories(include/)
include_directories(${SDL2_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(virtualmachine ${SRC_FILES})
target_link_libraries(virtualmachine PRIVATE SDL2::SDL2main)
target_link_libraries(virtualmachine PRIVATE SDL2::SDL2)
target_link_options(virtualmachine PRIVATE -static)
target_compile_features(virtualmachine PRIVATE cxx_std_20)