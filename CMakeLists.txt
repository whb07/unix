cmake_minimum_required(VERSION 3.20)
project(unix C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER  "clang-12")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wundef -Wpointer-arith -Wconversion")

add_library(lib
        src/malloc.c
        src/alloc.c
        src/bio.c
        src/bio.h)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)


add_executable(unix main.c)
