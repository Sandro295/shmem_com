cmake_minimum_required(VERSION 3.10)
project(SharedMemoryComm C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(producer producer.c common.h)
target_link_libraries(producer rt)

add_executable(consumer consumer.c common.h)
target_link_libraries(consumer rt)
