cmake_minimum_required(VERSION 3.10.2)
project(asyncLogger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_subdirectory(dependencies/fmt EXCLUDE_FROM_ALL)

include_directories(include)
include_directories(dependencies/fmt/include)


file(GLOB SRC ./src/*)

set(TESTS ON)
if (DEFINED TESTS)
    add_subdirectory(example)
endif()

add_library(asyncLogger SHARED ${SRC})
target_link_libraries(asyncLogger fmt-header-only pthread)
