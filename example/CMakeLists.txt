include_directories(../inlucde)
include_directories(../asyncLogger/include)
link_directories(../lib)

add_executable(echo.out EchoServer.cc)
target_link_libraries(echo.out tinyWebLib)

add_executable(timingWheel.out EchoServerWithTimingWheel.cc)
target_link_libraries(timingWheel.out tinyWebLib)

add_executable(time.out TimeServer.cc)
target_link_libraries(time.out tinyWebLib)

add_executable(timer.out timer.cc)
target_link_libraries(timer.out tinyWebLib)

add_executable(log.out logTest.cc)
target_link_libraries(log.out tinyWebLib)