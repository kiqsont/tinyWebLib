include_directories(../inlucde)
include_directories(../asyncLogger/include)
link_directories(../lib)

add_executable(test.out Echo.cc)
target_link_libraries(test.out tinyWebLib)