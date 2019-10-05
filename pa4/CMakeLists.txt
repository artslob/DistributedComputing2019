cmake_minimum_required(VERSION 3.7)
project(pa4)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -Werror")
set(SOURCE_FILES main.c ipc.c child.c log.c pipes.c process_common.c lamport.c)
add_executable(main ${SOURCE_FILES})
