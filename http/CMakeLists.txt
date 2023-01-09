include_directories(../include)
include_directories(../asyncLogger/include)
link_directories(../lib)

file(GLOB SRC "./*.cc")
add_executable(http.out ${SRC})
target_link_libraries(http.out tinyWebLib pthread rt)