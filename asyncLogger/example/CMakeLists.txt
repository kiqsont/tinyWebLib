include_directories(../include)
include_directories(../dependencies/fmt)

link_directories(../lib)
add_executable(test.out test.cc)
target_link_libraries(test.out asyncLogger)