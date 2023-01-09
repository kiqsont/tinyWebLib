include_directories(../include)
include_directories(../asyncLogger/include)
link_directories(../lib)

file(GLOB SRC "./*.cc")
add_executable(https.out ${SRC})
target_link_libraries(https.out tinyWebLib)