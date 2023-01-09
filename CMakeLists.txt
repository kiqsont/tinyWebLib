cmake_minimum_required(VERSION 3.10.2)
project(tinyWebLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(include)
include_directories(asyncLogger/include)
link_directories(asyncLogger/lib)

file(GLOB SRC ./src/*)

add_subdirectory(http)

set(TESTS ON)
if(DEFINED TESTS)
    add_subdirectory(example)
endif()

add_library(tinyWebLib SHARED ${SRC})
target_link_libraries(tinyWebLib asyncLogger)
target_link_libraries(tinyWebLib ssl crypto)
target_link_libraries(tinyWebLib pthread rt)