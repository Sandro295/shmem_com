cmake_minimum_required(VERSION 3.10)
project(LockFreeSHM)

add_executable(producer producer.c)
add_executable(consumer consumer.c)

target_link_libraries(producer rt)
target_link_libraries(consumer rt)
